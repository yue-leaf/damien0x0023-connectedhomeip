/**
*********************************************************************************************************
*               Copyright(c) 2021, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file      debug_port.h
* @brief
* @details
* @author
* @date      2021-4-6
* @version   v1.0
* *********************************************************************************************************
*/


#ifndef _DEBUG_PORT_H_
#define _DEBUG_PORT_H_

#ifdef __cplusplus
extern "C" {
#endif
#include "rtl876x.h"

typedef enum _t_debug_mode
{
    DIGI_DBG_SYSON,
    DIGI_DBG_BT,
    DIGI_DBG_PROCESSOR_SYS,
    DIGI_DBG_APB_ADC,
    DIGI_DBG_RTC,

    ANA_DBG_AUX_ADC = 0x10,
    ANA_DBG_RF_TRXIQ_AFE_TRMK_ADC,
    ANA_DBG_RF_TRANCERIVER
} T_DEBUG_MODE;


typedef enum _t_debug_pin_group
{
    DBG_PIN_GROUP_A,     //mapping to debug_pin_table_a
    DBG_PIN_GROUP_B,     //mapping to debug_pin_table_b
    DBG_PIN_GROUP_C,     //mapping to debug_pin_table_c
    DBG_PIN_GROUP_ALL_PAD = 0xff,
} T_PIN_GROUP;

/**
  * @brief  Open the debug port mode on specific pins
  * @param  red_table_mask: the pin bit map used in the table debug_pin_table_red.
  * @param  black_table_mask: the pin bit map used in the table debug_pin_table_black.
  * @param  debug_mode: debug mode select from T_DEBUG_MODE
  * @retval None
  */
extern void (*debug_port_open)(T_DEBUG_MODE debug_mode);

/**
  * @brief  Close the debug mode
  * @retval None
  */
void debug_port_close(void);

/**
  * @brief  set debug mode on specific pins
  * @param  pin_group: the pin group selected.
  *   This parameter can be any combination of the following values:
  *     @arg DBG_PIN_GROUP_A : the digital debug pin bit 0 ~ 31 could be selected as below table.
  *             ADC_0, ADC_1, ADC_2, ADC_3, P1_0, P1_1, P2_0,   P2_1,   //digi_debug_0 ~ 7
  *             P2_2,  P2_3,  P2_4,  P2_5,  P3_2, P3_3, MIC2_P, MIC2_N, //digi_debug_8 ~ 15
  *             MICBIAS,  LOUT_P,  LOUT_N,  P10_0                       //digi_debug_16 ~ 19
  *     @arg DBG_PIN_GROUP_B : the digital debug pin bit 0 ~ 31 could be selected as below table.
  *             MIC1_P,  MIC1_N,                                        //digi_debug_12 ~ 13
  * @retval None
  */
extern void (*debug_port_set_pin_bit_map)(T_PIN_GROUP pin_group, uint32_t dbg_bitmap);

void debug_port_bt_mac(bool is_set);

#ifdef __cplusplus
}
#endif

#endif /* _DEBUG_PORT_H_ */

