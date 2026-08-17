/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/**
*****************************************************************************************
* @file     tinyml_main.c
* @brief    Offline streaming KWS inference with a wake-word model.
* @date     2026-07-22
* @version  v2.1
*
* This sample drives a streaming keyword-spotting model with a STATIC PCM clip
* (kws_sample_pcm.c) instead of a live microphone. The clip is fed to the mel
* feature frontend frame-by-frame; the model is invoked once per 30ms step
* (3 x 10ms) and maintains its own ring-buffer state between invokes.
*
* Model: streaming wake-word model (see model_tflite.cpp header comment).
*   Input : int8[1,3,40]  scale=0.1019608  zero_point=-128
*   Output: uint8[1,1]    wake-word probability (dequantized by ts_realtek)
*   Frontend feeds RAW uint16 mel; quantize via uint16 * 0.383113 - 128.
*   Each invoke = 3 frames x 10ms = 30ms of audio.
*****************************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include <stdlib.h>
#include <stdbool.h>
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
#include "kws_frontend.h"

/*============================================================================*
 *                              Macros
 *============================================================================*/
#define TinyML_TASK_PRIORITY             2
#define TinyML_TASK_STACK_SIZE           512 * 20

/* KWS class labels - must match training order. Override at build time. */
#ifndef KWS_NUM_CLASSES
#define KWS_NUM_CLASSES                  2
#endif
#ifndef KWS_LABELS
#define KWS_LABELS                       { "hi_realtek", "noise" }
#endif

/* Detection threshold (wake word probability 0-1). */
#ifndef KWS_THRESHOLD
#define KWS_THRESHOLD                    0.80f
#endif

/* Index of the wake class in KWS_LABELS. */
#ifndef KWS_HIT_INDEX
#define KWS_HIT_INDEX                    0
#endif

/* Consecutive 30ms steps that must stay above KWS_THRESHOLD before firing. */
#ifndef KWS_REQUIRED_HITS
#define KWS_REQUIRED_HITS                3
#endif

/* Streaming model parameters (from model_tflite.cpp header comment):
 *   Input shape [1, 3, 40]: 3 time-steps (first_conv stride=3) x 40 mel channels.
 *   Each invoke advances the stream by 3 x 10ms = 30ms. */
#define KWS_FRAMES_PER_STEP              3
#define KWS_INPUT_BYTES                  (KWS_FRAMES_PER_STEP * KWS_NUM_CHANNELS)      /* 120 */
#define KWS_SAMPLES_PER_STEP             (KWS_FRAMES_PER_STEP * KWS_SAMPLES_PER_TICK)  /* 480 */

/* Input quantization - reproduces the exact training-time path.
 *
 * The TFLM microfrontend (kws_frontend) outputs RAW uint16 mel energies.
 * Training scales raw uint16 by 0.0390625 (= 1/25.6); the int8 model then
 * quantizes with scale=0.1019608, zero_point=-128.  On-device we apply both:
 *   int8 = round(uint16 * 0.0390625 / 0.1019608) - 128
 *        = round(uint16 * 0.383113) - 128  */
#define KWS_INP_SCALE_INV                (0.0390625f / 0.1019607857f)  /* ~= 0.383113 */
#define KWS_INP_ZP                       (-128)

/* ts_realtek_invoke dequantizes uint8 output to float32 internally;
 * caller receives probability directly (no manual scaling needed). */

/*============================================================================*
 *                              External data
 *============================================================================*/
/* Static PCM test clip (16 kHz, mono, int16) - see kws_sample_pcm.c. */
extern const int16_t      kws_sample_pcm[];
extern const unsigned int kws_sample_pcm_len;

/*============================================================================*
 *                              Variables
 *============================================================================*/
void *TinyML_task_handle;

/* Mel feature frontend state (in PSRAM to avoid stack pressure). */
static KwsFrontendState s_fe;

/* Model input: 3 frames x 40 channels = 120 int8 bytes. */
static int8_t s_features_int8[KWS_INPUT_BYTES];

/*============================================================================*
 *                              Functions
 *============================================================================*/
void TinyML_main_task(void *p_param);

/* Quantize one uint16 mel feature to int8. */
static inline int8_t quantize_feature(uint16_t v)
{
    int32_t q = (int32_t)((float)v * KWS_INP_SCALE_INV + 0.5f) + KWS_INP_ZP;
    if (q < -128) { q = -128; }
    if (q >  127) { q =  127; }
    return (int8_t)q;
}

/* -----------------------------------------------------------------------
 * ts_realtek engine: init once, invoke every 30ms step.
 * ----------------------------------------------------------------------- */
static int kws_engine_init(void)
{
    ts_realtek_register_heap(ts_malloc, ts_free);

    int sret = ts_realtek_set_arena_size(80u * 1024u);
    DBG_DIRECT("[TS] set_arena_size(80KB) ret=%d", sret);

    if (ts_realtek_register_model(get_model_pointer(), get_model_size()) != 0)
    {
        DBG_DIRECT("[TS] ERR: ts_realtek_register_model failed");
        return -1;
    }
    if (ts_realtek_init() != 0)
    {
        DBG_DIRECT("[TS] ERR: ts_realtek_init failed");
        return -2;
    }
    return 0;
}

/* Run one streaming inference step.
 * input: int8[KWS_INPUT_BYTES] = 3 quantized mel frames.
 * Returns wake-word probability in [0, 1], or -1.0f on error. */
static float kws_engine_invoke(const int8_t *input)
{
    float out_f = 0.0f;
    uint32_t out_actual = 0;
    int ret = ts_realtek_invoke(input, KWS_INPUT_BYTES,
                                &out_f, sizeof(out_f), &out_actual);
    if (ret != 0)
    {
        DBG_DIRECT("[TS] ERR: invoke failed: %d", ret);
        return -1.0f;
    }
    return out_f;  /* already dequantized by ts_realtek_invoke */
}

void TinyML_task_init()
{
    os_task_create(&TinyML_task_handle, "app", TinyML_main_task, 0,
                   TinyML_TASK_STACK_SIZE, TinyML_TASK_PRIORITY);
}

/**
 * @brief  Offline streaming KWS task: static PCM -> mel frontend -> TFLite-Micro.
 *
 * Feeds the embedded clip through the model one 30ms step at a time and
 * reports the wake-word probability trajectory. No microphone is used.
 */
void TinyML_main_task(void *p_param)
{
    static const char *labels[KWS_NUM_CLASSES] = KWS_LABELS;

    uint32_t actual_mhz = 0;
    pm_cpu_freq_set(125, &actual_mhz);
    DBG_DIRECT("CPU actual clock is %d MHz", actual_mhz);

    dwt_cyccnt_init();

    /* Initialize TFLM microfrontend (matches training: 40ch, 30ms win, 10ms step, PCAN). */
    if (KwsFrontendInit(&s_fe) != 1)
    {
        DBG_DIRECT("[KWS] ERR: KwsFrontendInit failed");
        return;
    }

    /* TFLite-Micro engine - initialised once, streaming state preserved across invokes. */
    if (kws_engine_init() != 0)
    {
        DBG_DIRECT("[KWS] ERR: kws_engine_init failed");
        return;
    }

    uint32_t num_steps = kws_sample_pcm_len / KWS_SAMPLES_PER_STEP;
    DBG_DIRECT("KWS offline start: %dHz, %d samples, %d steps (30ms/step)",
               KWS_SAMPLE_RATE, (int)kws_sample_pcm_len, (int)num_steps);

    uint8_t consecutive_hits = 0;
    int     best_score_pct   = 0;
    bool    woke             = false;

    for (uint32_t step = 0; step < num_steps; step++)
    {
        const int16_t *step_pcm = kws_sample_pcm + step * KWS_SAMPLES_PER_STEP;

        /* Extract 3 mel feature vectors (one per 10ms frame) and quantize. */
        for (int f = 0; f < KWS_FRAMES_PER_STEP; f++)
        {
            size_t n_read = 0;
            KwsFrontendOutput out = KwsFrontendProcess(
                                        &s_fe,
                                        step_pcm + f * KWS_SAMPLES_PER_TICK,
                                        KWS_SAMPLES_PER_TICK, &n_read);

            if (out.size == 0)
            {
                /* Frontend not ready yet (initial fill); fill zeros for this frame. */
                for (int ch = 0; ch < KWS_NUM_CHANNELS; ch++)
                {
                    s_features_int8[f * KWS_NUM_CHANNELS + ch] = (int8_t)KWS_INP_ZP;
                }
            }
            else
            {
                for (int ch = 0; ch < KWS_NUM_CHANNELS; ch++)
                {
                    s_features_int8[f * KWS_NUM_CHANNELS + ch] =
                        quantize_feature(out.values[ch]);
                }
            }
            (void)n_read;
        }

        /* Run one streaming inference step. */
        uint32_t t1    = read_cpu_counter();
        float    prob  = kws_engine_invoke(s_features_int8);
        uint32_t t_inf = (read_cpu_counter() - t1) / 125;  /* cycles -> us @125MHz */

        if (prob < 0.0f) { continue; }

        int score_pct = (int)(prob * 100.0f + 0.5f);
        if (score_pct > best_score_pct) { best_score_pct = score_pct; }

        if (prob >= KWS_THRESHOLD)
        {
            consecutive_hits++;
            if (consecutive_hits >= KWS_REQUIRED_HITS)
            {
                DBG_DIRECT("[KWS] *** WAKE UP*** %s (%d%%) step=%d inf=%dus",
                           labels[KWS_HIT_INDEX], score_pct, (int)step, (int)t_inf);
                woke = true;
                consecutive_hits = 0;
            }
            else
            {
                DBG_DIRECT("[KWS] pre-hit %d/%d %s (%d%%) step=%d inf=%dus",
                           (int)consecutive_hits, (int)KWS_REQUIRED_HITS,
                           labels[KWS_HIT_INDEX], score_pct, (int)step, (int)t_inf);
            }
        }
        else
        {
            consecutive_hits = 0;
            if (score_pct >= (int)(KWS_THRESHOLD * 100.0f / 2))
            {
                /* Log moderate scores for tuning (above half-threshold). */
                DBG_DIRECT("[KWS] %d%% step=%d inf=%dus", score_pct, (int)step, (int)t_inf);
            }
        }
    }

    DBG_DIRECT("[KWS] done: %d steps, best=%d%%, wake=%s",
               (int)num_steps, best_score_pct, woke ? "YES" : "NO");

    /* Release inference resources. */
    ts_realtek_deinit();
    KwsFrontendFree(&s_fe);

    while (true)
    {
    }
}
