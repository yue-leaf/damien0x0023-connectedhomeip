/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/*============================================================================*
 *               Define to prevent recursive inclusion
 *============================================================================*/
#ifndef __CLOCK_H_
#define __CLOCK_H_


/*============================================================================*
 *                               Header Files
*============================================================================*/
#include <stdint.h>
#include <stdbool.h>
#include "flash_nor_device.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup CLOCK Clock
  * @brief
  * @{
  */

/*============================================================================*
 *                              Variables
*============================================================================*/
/** @defgroup CLOCK_Exported_Variables Clock Exported Variables
  * @{
  */

/** @brief PLL clock source type enum. */
typedef enum
{
    CLK_NO_PLL,          /**< Not use PLL clock source.          */
    CLK_PLL1_SRC,        /**< Use PLL1 clock source.             */
    CLK_PLL2_SRC,        /**< Use PLL2 clock source.             */
    CLK_PLL_SRC_MAX,     /**< Max number of PLL clock source.    */
} PLL_CLK_SRC_TYPE;

/** @brief Clock source type enum. */
typedef enum
{
    CLK_40M,             /**< Use 40MHz clock source.              */
    CLK_PLL1_CPU,        /**< Use PLL1 CPU clock source.         */
    CLK_PLL1_PERI,       /**< Use PLL1 Peripheral clock source.  */
    CLK_PLL1_GMAC,       /**< Use PLL1 GMAC clock source.        */
    CLK_PLL1_GMAC50M,    /**< Use PLL1 GMAC50M clock source.     */
    CLK_PLL2,            /**< Use PLL2 clock source.         */
    CLK_SRC_MAX,         /**< Max number of clock source.    */
} CLK_SRC_TYPE;

/** End of CLOCK_Exported_Variables
  * @}
  */

/*============================================================================*
 *                              Functions
*============================================================================*/
/** @defgroup CLOCK_Exported_Functions Clock Exported Functions
  * @{
  */

/**
 * @brief Get clock source frequency.
 * @param[in]  clock_src_type        Clock source type.
 *
 * @return Clock source frequency.
*/
uint32_t pm_clock_src_freq_get(CLK_SRC_TYPE clock_src_type);

/**
 * @brief Set the CPU clock frequency.
 * @param[in]  required_mhz          Required frequency, unit: MHz.
 * @param[out] actual_mhz            Actual frequency after requirement, unit: MHz.
 *
 * @return  Status of operation.
 * @retval 0     Success.
 * @retval Others Fail to set the CPU clock frequency. This may happen when the require_mhz is not supported.
*/
int32_t pm_cpu_freq_set(uint32_t required_mhz, uint32_t *actual_mhz);

/**
 * @brief Retrieve the current CPU clock frequency.
 * @param[out]  current_mhz         The current CPU clock frequency in MHz.
 *
 * @return                          Operation result, and will always return 0.
 * @retval 0                        Success.
*/
int32_t pm_cpu_freq_get(uint32_t *current_mhz);

/**
 * @brief Set the CPU slow clock. After the system enters WFI, the CPU will automatically slow down to this frequency to reduce power consumption.
 *        - When clock source is CLK_40M, CPU slow clock frequency can be set to 40MHz divided by 1 to 1/64.
 *        - When clock source is CKO_PLL1_CPU, CPU slow clock frequency can be set to CKO_PLL1_CPU divided by 1 to 1/64.
 *        - When clock source is CKO_PLL2, CPU slow clock frequency can be set to CKO_PLL2 divided by 1 to 1/64.
 * @param[in]  required_mhz        Required CPU slow clock frequency.
 *
 * @return  Status of operation.
 * @retval 0      Success.
 * @retval Others Fail to set the CPU slow clock frequency. This may happen when the require_mhz is not supported.
*/
int32_t pm_cpu_slow_freq_set(uint32_t required_mhz);


/**
 * @brief Set the Flash Nor clock frequency.
 * @param[in]  idx                Corresponding SPIC ID for Flash Nor, refer to @ref FLASH_NOR_IDX_TYPE .
 * @param[in]  required_mhz       Required frequency, unit: MHz.
 * @param[out] actual_mhz         Actual frequency after requirement, unit: MHz.
 *
 * @return Status of Operation.
 * @retval True     Success.
 * @retval False    Fail to set the Flash Nor clock frequency. This may happen when the require_mhz is not supported.
*/
bool fmc_flash_nor_clock_switch(FLASH_NOR_IDX_TYPE idx, uint32_t required_mhz,
                                uint32_t *actual_mhz);

/**
 * @brief Set the PSRAM clock frequency.
 * @param[in]  idx               Corresponding SPIC ID for PSRAM, refer to @ref PSRAM_IDX_TYPE .
 * @param[in]  required_mhz      Required frequency, unit: MHz.
 * @param[out] actual_mhz        Actual frequency after requirement, unit: MHz.
 *
 * @return Status of Operation.
 * @retval True     Success.
 * @retval False    Fail to set the PSRAM clock frequency. This may happen when the require_mhz is not supported.
*/
bool fmc_psram_clock_switch(PSRAM_IDX_TYPE idx, uint32_t required_mhz, uint32_t *actual_mhz);


/**
 * @brief Set the frequency of the display's corresponding PLL clock source.
 * @param[in]  clk_src_type     PLL clock source selection: Choose the PLL clock source that needs to be initialized.
 * @param[in]  pll_output_mhz   PLL clock source frequency: The current frequency of the PLL2 clock source is fixed at 160MHz. Only when the PLL clock source is selected as PLL1, the frequency of CKO_PLL1_PERI can be modified (100MHz, 125MHz, 250MHz), unit: MHz.
 * @param[in]  required_mhz     Actual required frequency for display. Unit: MHz. For reference only.
 *
 * @return Status of Operation.
 * @retval 0      Success.
 * @retval Others Fail to set the frequency of the display's corresponding PLL clock source. This may happen when the require_mhz is not supported.
*/
int32_t pm_display_freq_set(PLL_CLK_SRC_TYPE clk_src_type, uint32_t pll_output_mhz,
                            uint32_t required_mhz);

/**
 * @brief Request to turn off the PLL clock source corresponding to the display. Only when all modules using the same PLL clock source are turned off, will the PLL clock source be turned off.
 * @param[in]  clk_src_type    PLL clock source for display.
 *
 * @return Status of Operation, and will always return 0.
 * @retval 0      Success.
*/
int32_t pm_display_freq_unset(PLL_CLK_SRC_TYPE clk_src_type);


/**
 * @brief Set the frequency of the PLL clock source corresponding to SDHC (SDIO Host Controller).
 * @param[in]  clk_src_type     PLL clock source selection: Choose the PLL clock source that needs to be initialized.
 * @param[in]  pll_output_mhz   PLL clock source frequency: The current frequency of the PLL2 clock source is fixed at 160MHz. Only when the PLL clock source is selected as PLL1, the frequency of CKO_PLL1_PERI can be modified (100MHz, 125MHz, 250MHz), unit: MHz.
 * @param[in]  required_mhz     Actual required frequency for SDHC. Unit: MHz. For reference only.
 *
 * @return Status of Operation.
 * @retval 0      Success.
 * @retval Others Fail to set the frequency of the PLL clock source corresponding to SDHC. This may happen when the require_mhz is not supported.
*/
int32_t pm_sdhc_freq_set(PLL_CLK_SRC_TYPE clk_src_type, uint32_t pll_output_mhz,
                         uint32_t required_mhz);

/**
 * @brief Request to turn off the PLL clock source corresponding to SDHC. Only when all modules using the same PLL clock source are turned off, will the PLL clock source be turned off.
 * @param[in]  clk_src_type   PLL clock source for SDHC.
 *
 * @return Status of Operation, and will always return 0.
 * @retval 0      Success.
*/
int32_t pm_sdhc_freq_unset(PLL_CLK_SRC_TYPE clk_src_type);


/**
 * @brief Set the frequency of the PLL clock source corresponding to PKE (Public Key Engine).
 * @param[in]  clk_src_type     PLL clock source selection: Choose the PLL clock source that needs to be initialized.
 * @param[in]  pll_output_mhz   PLL clock source frequency: The current frequency of the PLL2 clock source is fixed at 160MHz. Only when the PLL clock source is selected as PLL1, the frequency of CKO_PLL1_PERI can be modified (100MHz, 125MHz, 250MHz), unit: MHz.
 * @param[in]  required_mhz     Actual required frequency for PKE. Unit: MHz. For reference only.
 *
 * @return Status of Operation.
 * @retval 0      Success.
 * @retval Others Fail to set the frequency of the PLL clock source corresponding to PKE. This may happen when the require_mhz is not supported.
*/
int32_t pm_pke_freq_set(PLL_CLK_SRC_TYPE clk_src_type, uint32_t pll_output_mhz,
                        uint32_t required_mhz);

/**
 * @brief Request to turn off the PLL clock source corresponding to PKE. Only when all modules using the same PLL clock source are turned off, will the PLL clock source be turned off.
 * @param[in]  clk_src_type  PLL clock source for PKE.
 *
 * @return Status of Operation, and will always return 0.
 * @retval 0      Success.
*/
int32_t pm_pke_freq_unset(PLL_CLK_SRC_TYPE clk_src_type);


/**
 * @brief Set the frequency of the PLL clock source corresponding to Ethernet.
 * @param[in]  clk_src_type     PLL clock source selection: Choose the PLL clock source that needs to be initialized.
 * @param[in]  pll_output_mhz   PLL clock source frequency: The Ethernet clock source can only select CKO_PLL1_ETH (125MHz, 250MHz) or CKO_PLL1_ETH50M (50MHz), unit: MHz.
 * @param[in]  required_mhz     Actual required frequency for Ethernet. Unit: MHz. For reference only.
 *
 * @return Status of Operation.
 * @retval 0      Success.
 * @retval Others Fail to set the frequency of the PLL clock source corresponding to Ethernet. This may happen when the require_mhz is not supported.
*/
int32_t pm_ethernet_freq_set(PLL_CLK_SRC_TYPE clk_src_type, uint32_t pll_output_mhz,
                             uint32_t required_mhz);

/**
 * @brief Request to turn off the PLL clock source corresponding to Ethernet. Only when all modules using the same PLL clock source are turned off, will the PLL clock source be turned off.
 * @param[in]  clk_src_type  PLL clock source for Ethernet.
 *
 * @return Status of Operation, and will always return 0.
 * @retval 0      Success.
*/
int32_t pm_ethernet_freq_unset(PLL_CLK_SRC_TYPE clk_src_type);


/**
 * @brief Set the frequency of the PLL clock source corresponding to USB.
 * @param[in]  clk_src_type     PLL clock source selection: Choose the PLL clock source that needs to be initialized.
 * @param[in]  pll_output_mhz   PLL clock source frequency: The current frequency of the PLL2 clock source is fixed at 160MHz. Only when the PLL clock source is selected as PLL1, the frequency of CKO_PLL1_PERI can be modified (100MHz, 125MHz, 250MHz), unit: MHz.
 * @param[in]  required_mhz     Actual required frequency for USB. Unit: MHz. For reference only.
 *
 * @return Status of Operation.
 * @retval 0      Success.
 * @retval Others Fail to set the frequency of the PLL clock source corresponding to USB. This may happen when the require_mhz is not supported.
*/
int32_t pm_usb_freq_set(PLL_CLK_SRC_TYPE clk_src_type, uint32_t pll_output_mhz,
                        uint32_t required_mhz);

/**
 * @brief Request to turn off the PLL clock source corresponding to USB. Only when all modules using the same PLL clock source are turned off, will the PLL clock source be turned off.
 * @param[in]  clk_src_type  PLL clock source for USB.
 *
 * @return Status of Operation, and will always return 0.
 * @retval 0      Success.
*/
int32_t pm_usb_freq_unset(PLL_CLK_SRC_TYPE clk_src_type);


/**
 * @brief Set the frequency of the PLL clock source corresponding to Timer.
 * @param[in]  clk_src_type     PLL clock source selection: Choose the PLL clock source that needs to be initialized.
 * @param[in]  pll_output_mhz   PLL clock source frequency: The current frequency of the PLL2 clock source is fixed at 160MHz. Only when the PLL clock source is selected as PLL1, the frequency of CKO_PLL1_PERI can be modified (100MHz, 125MHz, 250MHz), unit: MHz.
 * @param[in]  required_mhz     Actual required frequency for Timer. Unit: MHz. For reference only.
 *
 * @return Status of Operation.
 * @retval 0      Success.
 * @retval Others Fail to set the frequency of the PLL clock source corresponding to Timer. This may happen when the require_mhz is not supported.
*/
int32_t pm_timer_freq_set(PLL_CLK_SRC_TYPE clk_src_type, uint32_t pll_output_mhz,
                          uint32_t required_mhz);

/**
 * @brief Request to turn off the PLL clock source corresponding to Timer. Only when all modules using the same PLL clock source are turned off, will the PLL clock source be turned off.
 * @param[in]  clk_src_type  PLL clock source for Timer.
 *
 * @return Status of Operation, and will always return 0.
 * @retval 0      Success.
*/
int32_t pm_timer_freq_unset(PLL_CLK_SRC_TYPE clk_src_type);


/**
 * @brief Set the frequency of the PLL clock source corresponding to Enhanced Timer.
 * @param[in]  clk_src_type     PLL clock source selection: Choose the PLL clock source that needs to be initialized.
 * @param[in]  pll_output_mhz   PLL clock source frequency: The current frequency of the PLL2 clock source is fixed at 160MHz. Only when the PLL clock source is selected as PLL1, the frequency of CKO_PLL1_PERI can be modified (100MHz, 125MHz, 250MHz), unit: MHz.
 * @param[in]  required_mhz     Actual required frequency for Enhanced Timer. Unit: MHz. For reference only.
 *
 * @return Status of Operation.
 * @retval 0      Success.
 * @retval Others Fail to set the frequency of the PLL clock source corresponding to Enhanced Timer. This may happen when the require_mhz is not supported.
*/
int32_t pm_enhanced_timer_freq_set(PLL_CLK_SRC_TYPE clk_src_type, uint32_t pll_output_mhz,
                                   uint32_t required_mhz);

/**
 * @brief Request to turn off the PLL clock source corresponding to Enhanced Timer. Only when all modules using the same PLL clock source are turned off, will the PLL clock source be turned off.
 * @param[in]  clk_src_type  PLL clock source for Enhanced Timer.
 *
 * @return Status of Operation, and will always return 0.
 * @retval 0      Success.
*/
int32_t pm_enhanced_timer_freq_unset(PLL_CLK_SRC_TYPE clk_src_type);

/**
 * @brief Set the frequency of the PLL clock source corresponding to IR (Infrared Radiation).
 * @param[in]  clk_src_type     PLL clock source selection: Choose the PLL clock source that needs to be initialized.
 * @param[in]  pll_output_mhz   PLL clock source frequency: The current frequency of the PLL2 clock source is fixed at 160MHz. Only when the PLL clock source is selected as PLL1, the frequency of CKO_PLL1_PERI can be modified (100MHz, 125MHz, 250MHz), unit: MHz.
 * @param[in]  required_mhz     Actual required frequency for IR. Unit: MHz. For reference only.
 *
 * @return Status of Operation.
 * @retval 0      Success.
 * @retval Others Fail to set the frequency of the PLL clock source corresponding to IR. This may happen when the require_mhz is not supported.
*/
int32_t pm_ir_freq_set(PLL_CLK_SRC_TYPE clk_src_type, uint32_t pll_output_mhz,
                       uint32_t required_mhz);

/**
 * @brief Request to turn off the PLL clock source corresponding to IR. Only when all modules using the same PLL clock source are turned off, will the PLL clock source be turned off.
 * @param[in]  clk_src_type  PLL clock source for IR.
 *
 * @return Status of Operation, and will always return 0.
 * @retval 0      Success.
*/
int32_t pm_ir_freq_unset(PLL_CLK_SRC_TYPE clk_src_type);

/**
 * @brief Set the frequency of the PLL clock source corresponding to SPI0 master.
 * @param[in]  clk_src_type     PLL clock source selection: Choose the PLL clock source that needs to be initialized.
 * @param[in]  pll_output_mhz   PLL clock source frequency: The current frequency of the PLL2 clock source is fixed at 160MHz. Only when the PLL clock source is selected as PLL1, the frequency of CKO_PLL1_PERI can be modified (100MHz, 125MHz, 250MHz), unit: MHz.
 * @param[in]  required_mhz     Actual required frequency for SPI0 master. Unit: MHz. For reference only.
 *
 * @return Status of Operation.
 * @retval 0      Success.
 * @retval Others Fail to set the frequency of the PLL clock source corresponding to SPI0 master. This may happen when the require_mhz is not supported.
*/
int32_t pm_spi0_master_freq_set(PLL_CLK_SRC_TYPE clk_src_type, uint32_t pll_output_mhz,
                                uint32_t required_mhz);

/**
 * @brief Request to turn off the PLL clock source corresponding to SPI0 master. Only when all modules using the same PLL clock source are turned off, will the PLL clock source be turned off.
 * @param[in]  clk_src_type  PLL clock source for SPI0 master.
 *
 * @return Status of Operation, and will always return 0.
 * @retval 0      Success.
*/
int32_t pm_spi0_master_freq_unset(PLL_CLK_SRC_TYPE clk_src_type);

/**
 * @brief Set the frequency of the PLL clock source corresponding to SPI0 slave.
 * @param[in]  clk_src_type     PLL clock source selection: Choose the PLL clock source that needs to be initialized.
 * @param[in]  pll_output_mhz   PLL clock source frequency: The current frequency of the PLL2 clock source is fixed at 160MHz. Only when the PLL clock source is selected as PLL1, the frequency of CKO_PLL1_PERI can be modified (100MHz, 125MHz, 250MHz), unit: MHz.
 * @param[in]  required_mhz     Actual required frequency for SPI0 slave. Unit: MHz. For reference only.
 *
 * @return Status of Operation.
 * @retval 0      Success.
 * @retval Others Fail to set the frequency of the PLL clock source corresponding to SPI0 slave. This may happen when the require_mhz is not supported.
*/
int32_t pm_spi0_slave_freq_set(PLL_CLK_SRC_TYPE clk_src_type, uint32_t pll_output_mhz,
                               uint32_t required_mhz);

/**
 * @brief Request to turn off the PLL clock source corresponding to SPI0 slave. Only when all modules using the same PLL clock source are turned off, will the PLL clock source be turned off.
 * @param[in]  clk_src_type  PLL clock source for SPI0 slave.
 *
 * @return Status of Operation, and will always return 0.
 * @retval 0      Success.
*/
int32_t pm_spi0_slave_freq_unset(PLL_CLK_SRC_TYPE clk_src_type);

/** End of CLOCK_Exported_Functions
  * @}
  */

/**
  * \cond     private
  * \defgroup CLOCK_Private_Functions
  * \{
  */

/**
 * @brief Config SPIC1 clock frequency. This API is deprecated, and user should use fmc_psram_clock_switch API.
 * @param  require_mhz  require SPIC1 frequency.
 * @param  real_mhz  the frequency of current SPIC1.
 * @return  Status of Operation.
 * @retval 0      Success
 * @retval Others Fail
*/
int32_t pm_spic1_freq_set(uint32_t required_mhz, uint32_t *actual_mhz);

/**
 * @brief Config SPIC2 clock frequency. This API is deprecated.
 * @param  require_mhz  require SPIC2 frequency.
 * @param  real_mhz  the frequency of current SPIC2.
 * @return  Status of Operation.
 * @retval 0      Success
 * @retval Others Fail
*/
int32_t pm_spic2_freq_set(uint32_t required_mhz, uint32_t *actual_mhz);

/**
 * @brief Configures the PLL2 output frequency with PPM correction.
 *
 * This function calculates and applies the integer and fractional divider parameters
 * for the Sigma-Delta Modulator (SDM) of PLL2. It takes a base frequency and applies
 * a Parts Per Million (PPM) offset to achieve high-precision frequency tuning.
 *
 * @note
 * 1. Double precision arithmetic is used to prevent precision loss, ensuring
 *    accurate calculation for fractional offsets (e.g., distinguishing 160.000160 MHz).
 * 2. The Fractional Offset Frequency (FOF) targets a 21-bit register width.
 * 3. Prerequisite: The PLL2 clock must be enabled before calling this function.
 *    Please ensure the appropriate clock interface API is called first to turn on PLL2.
 *
 * @param[in] required_base_mhz  The target base frequency in MHz (e.g., 160).
 * @param[in] ppm                Frequency deviation in Parts Per Million (PPM).
 *                               Positive values increase frequency, negative values decrease it.
 *
 * @return int32_t  0 on success, -1 if the calculated frequency is invalid.
 */
int32_t pm_clock_pll2_freq_set_with_ppm(uint32_t required_base_mhz, int8_t ppm);

/**
  *  End of CLOCK_Private_Functions
  * \}
  * \endcond
  */

/** End of CLOCK
  * @}
  */


#ifdef __cplusplus
}
#endif

#endif
