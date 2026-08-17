/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*============================================================================*
 *               Define to prevent recursive inclusion
 *============================================================================*/
#if ! defined (__CRC16BTX_H)
#define       __CRC16BTX_H

/*============================================================================*
 *                               Header Files
*============================================================================*/
#include <stdint.h>

/** @defgroup CRC_FCS CRC Implementation
  * @brief CRC implementation for specified polynomial: X**0 + X**2 + X**15 + X16
  * @{
  */

/*============================================================================*
 *                              Macro
*============================================================================*/
/** @defgroup CRC_FCS_Exported_Macros CRC Implementation Exported Macros
  * @{
  */
#define BTXFCS_INIT      0x0000  //!< Initial FCS value
#define BTXFCS_GOOD      0x0000  //!< Good final FCS value

/** End of CRC_FCS_Exported_Macros
  * @}
  */

/*============================================================================*
  *                                Functions
  *============================================================================*/
/** @defgroup CRC_FCS_Exported_Functions CRC Implementation Exported Functions
  * @{
  */
/**
* @brief  Calculate a new FCS given the current FCS and the new data.
*       Polynomial: X**0 + X**2 + X**15 + X**16
*
* @param  fcs: init or good final
* @param  cp: data point
* @param  len: length
*
* @return FCS value
*
*/
uint16_t btxfcs(uint16_t fcs,
                uint8_t  *cp,
                uint16_t len);

/**
* @brief Calculate the CRC16 IBM checksum for the given data and compare it to an expected checksum.
* @param  buf: data point
* @param  size: data size
* @param  compared_crc16: expected CRC16 result
* @return true: the same as compared_crc16, false: has different value
*
*/
bool crc16_ibm(uint8_t *buf, uint32_t size, uint16_t compared_crc16);

/**
* @brief  Proprietary interface for Realtek internal handshake API.
* @param  data: input data buffer
* @param  len: input data buffer length
* @return true if handshake pass, false otherwise
*
*/
extern bool btx_vendor_get_response(uint8_t *data, uint8_t len);

/** End of CRC_FCS_Exported_Functions
  * @}
  */

/** End of CRC_FCS
  * @}
  */

#endif  /**< #if ! defined (__CRC16BTX_H) */

/** End of CRC16BTX.H */
