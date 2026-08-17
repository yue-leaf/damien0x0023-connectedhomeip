/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef VECTOR_TABLE_H
#define VECTOR_TABLE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"
#include "stdbool.h"
#include "cmsis_compiler.h"
#include "vector_table_auto_gen.h"

/** @defgroup VECTOR_TABLE  Vector Table
  * @brief
  * @{
  */

/** @defgroup VECTOR_TABLE_Exported_Types Vector Table Exported Types
  * @brief
  * @{
  */

typedef void (*IRQ_Fun)(void);       /**< ISR Handler Prototype */

/* ================================================================================ */
/* ================                      IRQ                       ================ */
/* ================================================================================ */

typedef enum IRQn
{
    IRQn_AUTO_GEN
} IRQn_Type;

/* ================================================================================ */
/* ================                     VECTOR                     ================ */
/* ================================================================================ */

typedef enum
{
    VECTORn_AUTO_GEN
} VECTORn_Type;

extern void *RamVectorTable[];

#define GPIOA0_Handler    GPIO_A0_Handler
#define GPIOA1_Handler    GPIO_A1_Handle

/** End of VECTOR_TABLE_Exported_Types
  * @}
  */

/*============================================================================*
*                              Functions
*============================================================================*/
/** @defgroup VECTOR_TABLE_Exported_Functions Vector Table Exported Functions
  * @brief
  * @{
  */
/* @brief   Default interrupt handler used in ROM. */
extern void Default_Handler_rom(void);

/* @brief   Bluetooth MAC interrupt handler. */
extern void BTMAC_Handler(void);

/**
 * @brief  Initialize the RAM vector table at the specified RAM address.
 * @param[in]  ram_vector_addr The desired RAM address for the vector table.
 * @retval true if the initialization is successful, false otherwise.
 */
bool RamVectorTableInit(uint32_t ram_vector_addr);

/**
 * @brief  Update a specific Interrupt Service Routine (ISR) handler in the RAM vector table, should ensure that the ISR Handler runs RAM code.
 * @param[in]  v_num        Vector number.
 * @param[in]  isr_handler  User-defined ISR Handler.
 * @retval true if the update is successful, false otherwise.
 */
bool RamVectorTableUpdate(VECTORn_Type v_num, IRQ_Fun isr_handler);

/** End of VECTOR_TABLE_Exported_Functions
  * @}
  */

/** End of VECTOR_TABLE
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif // VECTOR_TABLE_H
