/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include "ppe_simulation.h"
#include "trace.h"
#include "stddef.h"
#include "rtl_ppe_int.h"

#define PPE_DBG     DBG_DIRECT

typedef struct
{
    uint8_t b;
    uint8_t g;
    uint8_t r;
    uint8_t a;
} T_ARGB;

uint32_t PPE_Get_Bytes_Per_Pixel_Result(PPE_ResultLayer_InitTypeDef *Result_layer)
{
    if (Result_layer->format == PPE_ARGB8888)
    {
        return 4;
    }
    else if (Result_layer->format == PPE_RGB888)
    {
        return 3;
    }
    else if (Result_layer->format == PPE_RGB565)
    {
        return 2;
    }
    else
    {
        PPE_DBG("this color format is not suuported in simulation yet !!\r\n");
        return 0;
    }
}

uint32_t PPE_Get_Bytes_Per_Pixel_Input(PPE_InputLayer_InitTypeDef *Input_layer)
{
    if (Input_layer->format == PPE_ARGB8888)
    {
        return 4;
    }
    else if (Input_layer->format == PPE_RGB888)
    {
        return 3;
    }
    else if (Input_layer->format == PPE_RGB565)
    {
        return 2;
    }
    else
    {
        PPE_DBG("this color format is not suuported in simulation yet !!\r\n");
        return 0;
    }
}

uint32_t Get_Pixel(uint8_t *source_address, uint32_t bytes_per_pixel)
{
    uint32_t data_temp = 0;
    for (uint32_t counter = 0; counter < bytes_per_pixel; counter++)
    {
        data_temp += (source_address[counter] << (counter * 8));
    }
    return data_temp;
}

uint32_t Convert_Pixel_Format_to_ARGB8888(uint32_t old_pixel, uint32_t bytes_per_pixel)
{
    uint32_t data_temp = 0;

    /*convert RGB565/RGB888/ARGB8888 to ARGB888*/
    if (bytes_per_pixel == 4)
    {
        data_temp = old_pixel;
    }
    else if (bytes_per_pixel == 3)
    {
        data_temp = old_pixel + 0xFF000000;                // extend the RGB888 with A=0xFF
    }
    else if (bytes_per_pixel == 2)
    {
        data_temp = ((old_pixel & 0x001F) << 3) + ((old_pixel & 0x001C) >> 2) + \
                    ((old_pixel & 0x07E0) << 5) + ((old_pixel & 0x0600) >> 1) + \
                    ((old_pixel & 0xF800) << 8) + ((old_pixel & 0xE000) << 3) + \
                    0xFF000000;                        // extend the  A=0xFF, fill the lsb with msb of each color
    }
    return data_temp;
}

PPE_ERR PPE_Scale_Simulation(ppe_buffer_t *image, ppe_buffer_t *buffer, float x_ratio,
                             float y_ratio)
{
    if (buffer == NULL)
    {
        return PPE_ERROR_NULL_TARGET;
    }
    if (image == NULL)
    {
        return PPE_ERROR_NULL_SOURCE;
    }
    PPE_InputLayer_InitTypeDef PPE_Input_Layer;
    PPE_ResultLayer_InitTypeDef PPE_Result_Layer;
    PPE_InitTypeDef PPE_Init_User;

    PPE_InputLayer_StructInit(&PPE_Input_Layer);
    PPE_ResultLayer_StructInit(&PPE_Result_Layer);
    PPE_structInit(&PPE_Init_User);
    if ((image->address % 4) || (buffer->address % 4))
    {
        return PPE_ERROR_ADDR_NOT_ALIGNED;
    }

    PPE_Input_Layer.src_addr                = (uint32_t)image->memory;
    PPE_Input_Layer.start_x                 = 0;
    PPE_Input_Layer.start_y                 = 0;
    PPE_Input_Layer.width                   = image->width;
    PPE_Input_Layer.height                  = image->height;
    PPE_Input_Layer.const_ABGR8888_value    = 0XFFFFFFFF;
    PPE_Input_Layer.format                  = image->format;
    PPE_Input_Layer.src                     = PPE_LAYER_SRC_FROM_DMA;
    PPE_Input_Layer.color_key_en            = DISABLE;
    PPE_Input_Layer.line_len                = PPE_Input_Layer.width;
    PPE_Input_Layer.key_color_value         = 0x0;
    PPE_Input_Layer.AXSIZE                  = 2; // 32bit bandwidth;
    PPE_Input_Layer.INCR                    = PPE_ARBURST_INCR;
    PPE_Input_Layer.AXCACHE                 = 1;
    PPE_Input_Layer.MAX_AXLEN_LOG           = PPE_MAX_AXLEN_127;
    PPE_Input_Layer.PRIOR                   = 0;
    PPE_Input_Layer.byte_swap               = PPE_NO_SWAP;
    PPE_Input_Layer.handshake_mode          = PPE_DMA_SW_HANDSHAKE;
    PPE_Input_Layer.polarity                = PPE_POLARITY_HIGH;
    PPE_Input_Layer.handshake_en            = DISABLE;
    PPE_Input_Layer.handshake_msize         = PPE_MSIZE_2;
    PPE_Input_Layer.hw_index                = 0;

    uint32_t fix_inv_sca_ratio_x = (uint32_t)(65536 / x_ratio); // 65536/ratio_x,ratio = 2
    uint32_t fix_inv_sca_ratio_y = (uint32_t)(65536 / y_ratio);
    buffer->width   = (uint32_t)PPE_Input_Layer.width * x_ratio;
    buffer->height  = (uint32_t)PPE_Input_Layer.height * y_ratio;

    PPE_Result_Layer.src_addr               = (uint32_t)buffer->memory;
    PPE_Result_Layer.width                  = buffer->width;
    PPE_Result_Layer.height                 = buffer->height;
    PPE_Result_Layer.format                 = buffer->format;
    PPE_Result_Layer.line_len               = PPE_Result_Layer.width;
    PPE_Result_Layer.AXSIZE                 = 2;    // 32bit bandwidth
    PPE_Result_Layer.INCR                   = PPE_ARBURST_INCR;
    PPE_Result_Layer.AXCACHE                = 1;
    PPE_Result_Layer.MAX_AXLEN_LOG          = PPE_MAX_AXLEN_127;
    PPE_Result_Layer.PRIOR                  = 0;
    PPE_Result_Layer.byte_swap              = PPE_NO_SWAP;
    PPE_Result_Layer.handshake_mode         = PPE_DMA_SW_HANDSHAKE;
    PPE_Result_Layer.polarity               = PPE_POLARITY_HIGH;
    PPE_Result_Layer.handshake_en           = DISABLE;
    PPE_Result_Layer.handshake_msize        = PPE_MSIZE_2;
    PPE_Result_Layer.hw_index               = 0;

    uint32_t scale_width  = PPE_Input_Layer.width * 65536 / fix_inv_sca_ratio_x;
    uint32_t scale_height = PPE_Input_Layer.height * 65536 / fix_inv_sca_ratio_y;
    if ((scale_width > PPE_Result_Layer.width) | (scale_height > PPE_Result_Layer.height))
    {
        PPE_DBG("[error]scale width or scale height is bigger than result layer size\r\n");
        return PPE_ERROR_INVALID_PARAM;
    }
    /*get the pixel format of each layer*/
    uint32_t Input_layer1_bytes_per_pixel         = PPE_Get_Bytes_Per_Pixel_Input(&PPE_Input_Layer);
    uint32_t Result_layer_bytes_per_pixel         = PPE_Get_Bytes_Per_Pixel_Result(&PPE_Result_Layer);

    /*get the data from Input layer1*/
    uint32_t result_ARGB8888_temp = 0;
    uint32_t Input_layer1_ARGB8888_temp0_0 = 0;
    uint32_t Input_layer1_ARGB8888_temp0_1 = 0;
    uint32_t Input_layer1_ARGB8888_temp1_0 = 0;
    uint32_t Input_layer1_ARGB8888_temp1_1 = 0;

    for (uint32_t vertical_counter = 0; vertical_counter < scale_height; vertical_counter++)
    {
        for (uint32_t horizontal_counter = 0; horizontal_counter < scale_width; horizontal_counter++)
        {
            uint32_t Fix_Input_layer1_horizontal_pos = horizontal_counter * fix_inv_sca_ratio_x;
            uint32_t Fix_Input_layer1_vertical_pos   = vertical_counter * fix_inv_sca_ratio_y;


            uint32_t Input_layer1_horizontal_pos = Fix_Input_layer1_horizontal_pos >> 16;
            uint32_t Inpput_layer1_vertical_pos = Fix_Input_layer1_vertical_pos >> 16;

            uint32_t x_ratio = Fix_Input_layer1_horizontal_pos & 0xFFFF;
            uint32_t y_ratio = Fix_Input_layer1_vertical_pos & 0xFFFF;

            /*get the 2*2 matrix from input layer*/
            uint32_t input_layer_matrix_addr = PPE_Input_Layer.src_addr + (Inpput_layer1_vertical_pos *
                                                                           PPE_Input_Layer.width + Input_layer1_horizontal_pos) * Input_layer1_bytes_per_pixel;
            /*get 0-0*/
            Input_layer1_ARGB8888_temp0_0 = Get_Pixel((uint8_t *)input_layer_matrix_addr,
                                                      Input_layer1_bytes_per_pixel);
            /*get 0-1*/
            if (Input_layer1_horizontal_pos == (PPE_Input_Layer.width - 1))
            {
                Input_layer1_ARGB8888_temp0_1 = Input_layer1_ARGB8888_temp0_0;
            }
            else
            {
                Input_layer1_ARGB8888_temp0_1 = Get_Pixel((uint8_t *)(input_layer_matrix_addr +
                                                                      Input_layer1_bytes_per_pixel), Input_layer1_bytes_per_pixel);
            }
            /*get 1-0*/
            if (Inpput_layer1_vertical_pos == (PPE_Input_Layer.height - 1))
            {
                Input_layer1_ARGB8888_temp1_0 = Input_layer1_ARGB8888_temp0_0;
            }
            else
            {
                Input_layer1_ARGB8888_temp1_0 = Get_Pixel((uint8_t *)(input_layer_matrix_addr +
                                                                      Input_layer1_bytes_per_pixel * PPE_Input_Layer.width), Input_layer1_bytes_per_pixel);
            }
            /*get 1-1*/
            if ((Input_layer1_horizontal_pos == (PPE_Input_Layer.width - 1)) &&
                !(Inpput_layer1_vertical_pos == (PPE_Input_Layer.height - 1)))
            {
                Input_layer1_ARGB8888_temp1_1 = Input_layer1_ARGB8888_temp1_0;
            }
            else if ((!(Input_layer1_horizontal_pos == (PPE_Input_Layer.width - 1))) &&
                     (Inpput_layer1_vertical_pos == (PPE_Input_Layer.height - 1)))
            {
                Input_layer1_ARGB8888_temp1_1 = Input_layer1_ARGB8888_temp0_1;
            }
            else if (((Input_layer1_horizontal_pos == (PPE_Input_Layer.width - 1))) &&
                     (Inpput_layer1_vertical_pos == (PPE_Input_Layer.height - 1)))
            {
                Input_layer1_ARGB8888_temp1_1 = Input_layer1_ARGB8888_temp0_0;
            }
            else
            {
                Input_layer1_ARGB8888_temp1_1 = Get_Pixel((uint8_t *)(input_layer_matrix_addr +
                                                                      Input_layer1_bytes_per_pixel * (PPE_Input_Layer.width + 1)), Input_layer1_bytes_per_pixel);
            }
            /*convert the matrix into ARGB8888*/
            Input_layer1_ARGB8888_temp0_0 = Convert_Pixel_Format_to_ARGB8888(Input_layer1_ARGB8888_temp0_0,
                                                                             Input_layer1_bytes_per_pixel);
            Input_layer1_ARGB8888_temp0_1 = Convert_Pixel_Format_to_ARGB8888(Input_layer1_ARGB8888_temp0_1,
                                                                             Input_layer1_bytes_per_pixel);
            Input_layer1_ARGB8888_temp1_0 = Convert_Pixel_Format_to_ARGB8888(Input_layer1_ARGB8888_temp1_0,
                                                                             Input_layer1_bytes_per_pixel);
            Input_layer1_ARGB8888_temp1_1 = Convert_Pixel_Format_to_ARGB8888(Input_layer1_ARGB8888_temp1_1,
                                                                             Input_layer1_bytes_per_pixel);

            /*change the alpha of data from memory*/ // this should be cancelled when simulating the data from const value register
            Input_layer1_ARGB8888_temp0_0 = (Input_layer1_ARGB8888_temp0_0 & 0xFFFFFF) + (((
                    Input_layer1_ARGB8888_temp0_0 >> 24) * (PPE_Input_Layer.const_ABGR8888_value >> 24) / 255) << 24);
            Input_layer1_ARGB8888_temp0_1 = (Input_layer1_ARGB8888_temp0_1 & 0xFFFFFF) + (((
                    Input_layer1_ARGB8888_temp0_1 >> 24) * (PPE_Input_Layer.const_ABGR8888_value >> 24) / 255) << 24);
            Input_layer1_ARGB8888_temp1_0 = (Input_layer1_ARGB8888_temp1_0 & 0xFFFFFF) + (((
                    Input_layer1_ARGB8888_temp1_0 >> 24) * (PPE_Input_Layer.const_ABGR8888_value >> 24) / 255) << 24);
            Input_layer1_ARGB8888_temp1_1 = (Input_layer1_ARGB8888_temp1_1 & 0xFFFFFF) + (((
                    Input_layer1_ARGB8888_temp1_1 >> 24) * (PPE_Input_Layer.const_ABGR8888_value >> 24) / 255) << 24);

            T_ARGB *Old_Pixel_0_0 = (T_ARGB *)(&Input_layer1_ARGB8888_temp0_0);
            T_ARGB *Old_Pixel_0_1 = (T_ARGB *)(&Input_layer1_ARGB8888_temp0_1);
            T_ARGB *Old_Pixel_1_0 = (T_ARGB *)(&Input_layer1_ARGB8888_temp1_0);
            T_ARGB *Old_Pixel_1_1 = (T_ARGB *)(&Input_layer1_ARGB8888_temp1_1);
            /*calculate the result pixel*/
            T_ARGB calculate_result;

            calculate_result.a = (((65536 - x_ratio) * ((((65536 - y_ratio) * Old_Pixel_0_0->a) +
                                                         (y_ratio * Old_Pixel_1_0->a)) >> 16))    + \
                                  (x_ratio  * ((((65536 - y_ratio) * Old_Pixel_0_1->a) + (y_ratio * Old_Pixel_1_1->a)) >> 16))) >> 16;
            calculate_result.r = (((65536 - x_ratio) * ((((65536 - y_ratio) * Old_Pixel_0_0->r) +
                                                         (y_ratio * Old_Pixel_1_0->r)) >> 16))    + \
                                  (x_ratio  * ((((65536 - y_ratio) * Old_Pixel_0_1->r) + (y_ratio * Old_Pixel_1_1->r)) >> 16))) >> 16;
            calculate_result.g = (((65536 - x_ratio) * ((((65536 - y_ratio) * Old_Pixel_0_0->g) +
                                                         (y_ratio * Old_Pixel_1_0->g)) >> 16))    + \
                                  (x_ratio  * ((((65536 - y_ratio) * Old_Pixel_0_1->g) + (y_ratio * Old_Pixel_1_1->g)) >> 16))) >> 16;
            calculate_result.b = (((65536 - x_ratio) * ((((65536 - y_ratio) * Old_Pixel_0_0->b) +
                                                         (y_ratio * Old_Pixel_1_0->b)) >> 16))    + \
                                  (x_ratio  * ((((65536 - y_ratio) * Old_Pixel_0_1->b) + (y_ratio * Old_Pixel_1_1->b)) >> 16))) >> 16;

            result_ARGB8888_temp = (*(uint32_t *)(&calculate_result));

            /*convert the ARGB8888 into desired output format and write it to memory*/
            uint32_t result_layer_offset_now  = PPE_Result_Layer.src_addr + ((vertical_counter +
                                                                              PPE_Input_Layer.start_y) * PPE_Result_Layer.width + (horizontal_counter + PPE_Input_Layer.start_x))
                                                * Result_layer_bytes_per_pixel;
            uint8_t *byte_pointer_temp = (uint8_t *)result_layer_offset_now;
            if (Result_layer_bytes_per_pixel == 4)
            {
                /*output ARGB8888*/
                byte_pointer_temp[0] = (result_ARGB8888_temp >> 0) & 0xFF;
                byte_pointer_temp[1] = (result_ARGB8888_temp >> 8) & 0xFF;
                byte_pointer_temp[2] = (result_ARGB8888_temp >> 16) & 0xFF;
                byte_pointer_temp[3] = (result_ARGB8888_temp >> 24) & 0xFF;
            }
            else if (Result_layer_bytes_per_pixel == 3)
            {
                /*output RGB888*/
                byte_pointer_temp[0] = (result_ARGB8888_temp >> 0) & 0xFF;
                byte_pointer_temp[1] = (result_ARGB8888_temp >> 8) & 0xFF;
                byte_pointer_temp[2] = (result_ARGB8888_temp >> 16) & 0xFF;
            }
            else if (Result_layer_bytes_per_pixel == 2)
            {
                /*output RGB565*/
                byte_pointer_temp[0] = (((result_ARGB8888_temp >> 0) & 0xF8) >> 3) + (((
                                                                                           result_ARGB8888_temp >> 8)  & 0x1C) << 3);
                byte_pointer_temp[1] = (((result_ARGB8888_temp >> 8) & 0xE0) >> 5) + (((
                                                                                           result_ARGB8888_temp >> 16) & 0xF8));
            }
        }
    }
    return PPE_SUCCESS;
}
