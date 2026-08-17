/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */
/* Demo includes. */
#include "rtl876x.h"
#include "rtl876x_lib_platform.h"
#include "rtl_pinmux.h"
#include "rtl_gdma.h"
#include "rtl_idu.h"
#include "rtl_rcc.h"
#include "idu_test_source.h"
#include "trace.h"
/*-----------------------------------------------------------*/

static uint8_t __attribute__((aligned(4))) buf[9000];

void IDU_test(void)
{
    RCC_PeriphClockCmd(APBPeriph_IDU, APBPeriph_IDU_CLOCK, ENABLE);
    IDU_file_header *header = (IDU_file_header *)file_data;

    DBG_DIRECT("algo = %d, blur %d, YUV %d", header->algorithm_type.algorithm,
               header->algorithm_type.feature_2, header->algorithm_type.feature_1);
    DBG_DIRECT("width %d height %d compressed data size %d", header->raw_pic_width,
               header->raw_pic_height, file_len);

    IDU_decode_range range;
    range.start_column = 0;
    range.end_column = header->raw_pic_width - 1;
    range.start_line = 0;
    range.end_line = header->raw_pic_height - 1;
    range.target_stride = header->raw_pic_width * IDU_Get_Pixel_Size(header);
    IDU_DMA_config dma_cfg;
    dma_cfg.output_buf = (uint32_t *)buf;
    dma_cfg.RX_DMA_channel_num = 0;
    dma_cfg.TX_DMA_channel_num = 1;
    dma_cfg.RX_FIFO_INT_threshold = 8;
    dma_cfg.TX_FIFO_INT_threshold = 8;

    IDU_Decode(file_data, &range, &dma_cfg);

    for (uint32_t counter = 0; counter < sizeof(decode_file_data); counter++)
    {
        if (buf[counter] == decode_file_data[counter])
        {
            continue;
        }
        else
        {
            DBG_DIRECT("Received word is = %x\r\n", buf[counter]);
            DBG_DIRECT("Golden word is = %x\r\n", decode_file_data[counter]);
            DBG_DIRECT("[ERROR] occurs during decompress procedure at offset %x \r\n", counter);
            return;
        }
    }
    DBG_DIRECT("decompress procedure succeeded");
    /*decompress procedure finish*/
}

/*-----------------------------------------------------------*/
