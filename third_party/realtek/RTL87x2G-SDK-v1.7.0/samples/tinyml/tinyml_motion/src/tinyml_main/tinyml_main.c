/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/*
 * TinyMotion - MCU inference entry.
 *
 * End-to-end 1D-CNN motion trajectory recognition. There is NO hand-crafted
 * DSP feature extraction (no Edge Impulse code): a raw accelerometer window
 * is z-score normalized and int8-quantized in plain C, fed to the int8 CNN
 * via the ts_realtek TFLite-Micro engine, and the int8 softmax output is
 * decoded to a class with a confidence-threshold open-set (OOD) gate.
 *
 *   raw window (125 x 3 float, m/s^2, interleaved [x0,y0,z0,x1,...])
 *        -> motion_preprocess_quantize()   [z-score + int8 quant]
 *        -> ts_realtek_invoke()            [int8 CNN, 375B in -> float softmax out]
 *        -> motion_classify()              [argmax + reject-if-low-confidence]
 *
 * Note: ts_realtek_invoke() dequantizes the int8 softmax output tensor to
 * float32 internally (using the tensor's own scale/zero-point), so the caller
 * receives ready-to-use probabilities and no separate dequantize step is needed.
 */

/* Includes ------------------------------------------------------------------*/
#include <stdlib.h>
#include <stdio.h>
#include "cmsis_compiler.h"
#include "trace.h"
#include "ts_realtek.h"
#include "os_sched.h"
#include "ts_mem.h"
#include "pm.h"

#include "model_tflite.h"
#include <os_msg.h>
#include <os_task.h>
#include <app_msg.h>
#include "tinyml_main.h"
#include "utils.h"
#include "app_section.h"
#include "motion_preprocess.h"
#include "motion_demo_samples.h"

/*============================================================================*
 *                              Macros
 *============================================================================*/
#define TinyML_TASK_PRIORITY             2          //!< Task priorities
#define TinyML_TASK_STACK_SIZE           512 * 10    //!< Task stack size

/*============================================================================*
 *                              Variables
 *============================================================================*/
void *TinyML_task_handle;   //!< APP Task handle

pfunc ts_timer_timeout_func = NULL;

/* Scratch buffers (static to keep them off the task stack). */
static int8_t  g_input_q[MOTION_TOTAL_RAW];   /* 375 int8 model input   */
static float   g_probs[MOTION_N_CLASSES];     /* 4   softmax probs (dequantized by ts_realtek) */

/*============================================================================*
 *                              Functions
 *============================================================================*/
void TinyML_main_task(void *p_param);

/* Forward TFLite-Micro / ts_realtek log messages to the debug UART. This is
 * what surfaces the exact reason for an init failure (unsupported op, schema
 * version, allocation size, ...). */
static void ts_log_cb(const char *msg)
{
    DBG_DIRECT("[TSLOG] %s", msg);
}

/* -----------------------------------------------------------------------
 * ts_run_inference_int8 - run the int8 CNN on a pre-quantized int8 window.
 * Direct ts_realtek API, no TFLite headers required. The TinyMotion model
 * is fully int8 (input int8 [1,125,3], output int8 [1,4]); ts_realtek_invoke
 * dequantizes the int8 output tensor to float32 softmax probabilities and
 * writes them into out_probs, so out_len must be >= MOTION_N_CLASSES floats.
 * ----------------------------------------------------------------------- */
static int ts_run_inference_int8(const int8_t *input_q, uint32_t input_len,
                                 float *out_probs, uint32_t out_len)
{
    ts_realtek_register_heap(ts_malloc, ts_free);
    ts_realtek_register_log(ts_log_cb);
    ts_realtek_register_cpu_freq_hz(125000000);

    /* Small CNN (~25 KB flatbuffer). 80 KB arena fits inside the 100 KB PSRAM
     * heap (configTOTAL_psRAM_HEAP_SIZE) with room for the interpreter object;
     * the default 150 KB would overflow that heap after TS init. */
    ts_realtek_set_arena_size(80 * 1024);

    /* --- Diagnostics (buffer-only, no init required). Print once; the demo
     *     runs several inferences and we don't want to repeat this block. --- */
    static bool s_diag_printed = false;
    if (!s_diag_printed)
    {
        s_diag_printed = true;
        ts_realtek_model_info_t info;
        int mi = ts_realtek_get_model_info(get_model_pointer(), get_model_size(), &info);
        if (mi == 0)
        {
            DBG_DIRECT("[TM] model_info: in_type=%d out_type=%d in_bytes=%d out_bytes=%d",
                       (int)info.input_type, (int)info.output_type,
                       (int)info.input_bytes, (int)info.output_bytes);
            DBG_DIRECT("[TM] model_info: in_elems=%d out_elems=%d in_zp=%d out_zp=%d",
                       (int)info.input_elem_count, (int)info.output_elem_count,
                       (int)info.input_zero_point, (int)info.output_zero_point);
        }
        else
        {
            DBG_DIRECT("[TM] ERR: ts_realtek_get_model_info failed: %d", mi);
        }

        char ops[256];
        if (ts_realtek_get_unique_ops(get_model_pointer(), get_model_size(),
                                      ops, sizeof(ops)) == 0)
        {
            DBG_DIRECT("[TM] model ops: %s", ops);
        }
    }

    if (ts_realtek_register_model(get_model_pointer(), get_model_size()) != 0)
    {
        DBG_DIRECT("[TM] ERR: ts_realtek_register_model failed");
        return -1;
    }
    int init_ret = ts_realtek_init();
    if (init_ret != 0)
    {
        DBG_DIRECT("[TM] ERR: ts_realtek_init failed, err=%d", init_ret);
        return -2;
    }

    uint32_t out_actual_len = 0;
    int ret = ts_realtek_invoke(input_q, input_len,
                                out_probs, out_len, &out_actual_len);
    if (ret != 0)
    {
        ts_realtek_deinit();
        DBG_DIRECT("[TM] ERR: ts_realtek_invoke failed: %d", ret);
        return -3;
    }

    DBG_DIRECT("[TM] Inference cycles: %d", (unsigned int)ts_realtek_last_invoke_cycles());
    DBG_DIRECT("[TM] Output bytes: %d", (unsigned int)out_actual_len);

    ts_realtek_deinit();
    return 0;
}

void TinyML_task_init(void)
{
    DBG_DIRECT("DBG: TinyML_task_init enter");
    os_task_create(&TinyML_task_handle, "mlT", TinyML_main_task, 0, TinyML_TASK_STACK_SIZE,
                   TinyML_TASK_PRIORITY);
    DBG_DIRECT("DBG: TinyML_task_init done");
}

/* Active test window is selected via MOTION_TEST_CASE in motion_demo_samples.h.
 * Default: MOTION_TEST_CASE_WAVE (3).  Override with -DMOTION_TEST_CASE=<0-4>. */

/**
 * @brief   App task: run one TinyMotion inference on the demo window.
 * @param   p_param  unused
 */
void TinyML_main_task(void *p_param)
{
    DBG_DIRECT("DBG: TinyML_main_task enter");
    uint32_t actual_mhz = 0;
    pm_cpu_freq_set(125, &actual_mhz);
    DBG_DIRECT("CPU actual clock is %d MHz", actual_mhz);

    dwt_cyccnt_init();

    DBG_DIRECT("[TM] Test case: %s (expected -> %s)", DEMO_CASE_LABEL,
               (DEMO_CASE_EXPECTED_CLS == MOTION_CLASS_UNKNOWN) ? "unknown (rejected)" :
               MOTION_CLASS_NAMES[DEMO_CASE_EXPECTED_CLS]);

    /* 1) Preprocess: z-score + int8 quantization (self-written, no EI code). */
    uint32_t t0 = read_cpu_counter();
    motion_preprocess_quantize(g_demo_window, g_input_q);
    uint32_t t1 = read_cpu_counter();
    DBG_DIRECT("[TM] Preprocess time: %d us", (t1 - t0) / 125);

    /* 2) Inference: int8 CNN via ts_realtek (375B int8 in -> float softmax out).
     *    ts_realtek_invoke dequantizes the int8 output to float internally, so
     *    the buffer must hold MOTION_N_CLASSES floats (not int8 bytes). */
    int infer_ret = ts_run_inference_int8(g_input_q, MOTION_TOTAL_RAW,
                                          g_probs, MOTION_N_CLASSES * sizeof(float));
    if (infer_ret != 0)
    {
        DBG_DIRECT("[TM] ERR: tflite model infer failed, ret = %d", infer_ret);
        while (true) { }
    }

    /* 3) Decode: g_probs already holds float softmax probs (dequantized inside
     *    ts_realtek_invoke). Print as 0.xxxx (no %f dependency). */
    for (int c = 0; c < MOTION_N_CLASSES; ++c)
    {
        int p_i = (int)(g_probs[c] * 10000.0f + 0.5f);
        DBG_DIRECT("  probs[%d] %s = 0.%04d", c, MOTION_CLASS_NAMES[c], p_i);
    }

    /* 4) Classify with open-set (confidence) gate. */
    float conf = 0.0f;
    int cls = motion_classify(g_probs, &conf);
    int conf_i = (int)(conf * 10000.0f + 0.5f);
    const char *result_str;
    if (cls == MOTION_CLASS_UNKNOWN)
    {
        result_str = "unknown (rejected)";
        DBG_DIRECT("[TM] RESULT: unknown (rejected), max conf = 0.%04d", conf_i);
    }
    else
    {
        result_str = MOTION_CLASS_NAMES[cls];
        DBG_DIRECT("[TM] RESULT: class = %s, conf = 0.%04d",
                   MOTION_CLASS_NAMES[cls], conf_i);
    }
    if (cls == DEMO_CASE_EXPECTED_CLS)
    {
        DBG_DIRECT("[TM] PASS: got expected result '%s'", result_str);
    }
    else
    {
        DBG_DIRECT("[TM] FAIL: expected '%s', got '%s'", DEMO_CASE_LABEL, result_str);
    }

    while (true)
    {
    }
}
