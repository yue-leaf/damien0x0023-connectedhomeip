/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/*============================================================================*
 *                      Define to prevent recursive inclusion
 *============================================================================*/
#ifndef SECURE_APP_SECTION_H
#define SECURE_APP_SECTION_H


/** @defgroup APP_SECTION APP Section
  * @brief memory section definition for user application.
  * @{
  */

/*============================================================================*
 *                              Macro
*============================================================================*/
/** @defgroup APP_SECTION_Exported_Macros APP Section Sets Exported Macros
    * @{
    */
/** @defgroup APP_FLASH_SECTION APP Flash Section
    *@brief  const data or flash function
    * @{
    */
#define APP_FLASH_HEADER           __attribute__((section(".app.flash.header")))     __attribute__((used))
#define APP_FLASH_HEADER_EXT       __attribute__((section(".app.flash.header_ext"))) __attribute__((used))
#define APP_FLASH_START_SECTION    __attribute__((section(".app.flash.start")))      __attribute__((used))
#define APP_FLASH_TEXT_SECTION     __attribute__((section(".app.flash.text")))
#define APP_FLASH_RODATA_SECTION   __attribute__((section(".app.flash.rodata")))

#define ISR_TEXT_SECTION           __attribute__((section(".isr.text"))) /*not very urgent isr*/
/** @} */

/** @defgroup APP_RAM_SECTION RAM Data Section
    *@brief  global variable or ram function,  data on (default)
    * @{
    */
#define APP_RAM_DATA_SECTION       __attribute__((section(".app.ram.data")))
#define EXT_DATA_SRAM_SECTION      __attribute__((section(".app.ext.datasram.data")))

#define RAM_DATA                   APP_RAM_DATA_SECTION
#define EXT_RAM_DATA               EXT_DATA_SRAM_SECTION
/** @} */

/** @} */

/** @defgroup APP_FUNCTION_SECTION RAM Function Section Sets
    *@brief  ram code
    * @{
    */
#define APP_ITCM1_RAM_TEXT_SECTION __attribute__((section(".app.itcm1.ram.text")))
#define APP_RAM_START_SECTION      __attribute__((section(".app.ram.start")))
#define APP_RAM_TEXT_SECTION       __attribute__((section(".app.ram.text")))

#define RAM_TEXT_SECTION           APP_ITCM1_RAM_TEXT_SECTION
#define RAM_FUNCTION               APP_RAM_TEXT_SECTION


/* app encryption */
#define APP_ENCRYPTION_SIGNATURE   __attribute__((section(".encryption.signature")))
#define APP_ENCRYPION_DUMMY_ALIGN  __attribute__((section(".encryption.dummy.align"))) __attribute__((aligned(16)))
/* app encryption code*/
#define APP_ENCRYPTION_TEXT_SECTION __attribute__((section(".app.encryption.text")))



/** @} */



/** @} */ /* End of group APP_SECTION */



#endif // APP_SECTION_H
