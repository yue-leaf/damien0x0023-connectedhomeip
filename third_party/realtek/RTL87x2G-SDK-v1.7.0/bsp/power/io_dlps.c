/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include <stddef.h>
#include "app_section.h"
#include "board.h"
#include "pm.h"
#include "mem_config.h"
#include "io_dlps.h"
#include "rtl_pinmux.h"
#include "rtl_nvic.h"
#include "rtl_rcc.h"
#include "rtl876x_lib_platform.h"
#include "fmc_api.h"
#include "utils.h"
#include "trace.h"
#include "wdt.h"

/*============================================================================*
 *                         IO DLPS
 *============================================================================*/

/******************************************************************************/
/******************** [PINMUX DLPS] *******************************************/
/******************************************************************************/
/******************************************************************************/

volatile PINMUXStoreReg_Typedef Pinmux_StoreReg;

extern void Pinmux_DLPSEnter(void *PeriReg, void *StoreBuf) RAM_FUNCTION;
extern void Pinmux_DLPSExit(void *PeriReg, void *StoreBuf) RAM_FUNCTION;

/******************************************************************************/
/******************** [ADC DLPS] **********************************************/
/******************************************************************************/
/******************************************************************************/
#if USE_ADC_DLPS
#include "rtl_adc.h"

volatile ADCStoreReg_TypeDef ADC_StoreReg;

extern void ADC_DLPSEnter(void *PeriReg, void *StoreBuf) RAM_FUNCTION;
extern void ADC_DLPSExit(void *PeriReg, void *StoreBuf) RAM_FUNCTION;

#endif

/******************************************************************************/
/******************** [GPIO DLPS] *********************************************/
/******************************************************************************/
/******************************************************************************/
#if USE_GPIOA_DLPS || USE_GPIOB_DLPS
#include "rtl_gpio.h"

#if USE_GPIOA_DLPS
volatile GPIOStoreReg_Typedef GPIOA_StoreReg;
#endif
#if USE_GPIOB_DLPS
volatile GPIOStoreReg_Typedef GPIOB_StoreReg;
#endif

extern void GPIO_DLPSEnter(void *PeriReg, void *StoreBuf) RAM_FUNCTION;
extern void GPIO_DLPSExit(void *PeriReg, void *StoreBuf) RAM_FUNCTION;

#endif  /* USE_GPIOA_DLPS || USE_GPIOB_DLPS */

/******************************************************************************/
/******************** [I2C DLPS] **********************************************/
/******************************************************************************/
/******************************************************************************/
#if USE_I2C0_DLPS || USE_I2C1_DLPS || USE_I2C2_DLPS || USE_I2C3_DLPS
#include "rtl_i2c.h"

#if USE_I2C0_DLPS
volatile I2CStoreReg_Typedef I2C0_StoreReg;
#endif
#if USE_I2C1_DLPS
volatile I2CStoreReg_Typedef I2C1_StoreReg;
#endif
#if USE_I2C2_DLPS
volatile I2CStoreReg_Typedef I2C2_StoreReg;
#endif
#if USE_I2C3_DLPS
volatile I2CStoreReg_Typedef I2C3_StoreReg;
#endif

extern void I2C_DLPSEnter(void *PeriReg, void *StoreBuf) RAM_FUNCTION;
extern void I2C_DLPSExit(void *PeriReg, void *StoreBuf) RAM_FUNCTION;

#endif  /* USE_I2C0_DLPS || USE_I2C1_DLPS || USE_I2C2_DLPS || USE_I2C3_DLPS */


/******************************************************************************/
/******************** [IR DLPS] ***********************************************/
/******************************************************************************/
/******************************************************************************/
#if USE_IR_DLPS
#include "rtl_ir.h"

volatile IRStoreReg_Typedef IR_StoreReg;

extern void IR_DLPSEnter(void *PeriReg, void *StoreBuf) RAM_FUNCTION;
extern void IR_DLPSExit(void *PeriReg, void *StoreBuf) RAM_FUNCTION;

#endif

/******************************************************************************/
/******************** [KEYSCAN DLPS] ******************************************/
/******************************************************************************/
/******************************************************************************/
#if USE_KEYSCAN_DLPS
#include "rtl_keyscan.h"

volatile KEYSCANStoreReg_Typedef KeyScan_StoreReg;

extern void KEYSCAN_DLPSEnter(void *PeriReg, void *StoreBuf) RAM_FUNCTION;
extern void KEYSCAN_DLPSExit(void *PeriReg, void *StoreBuf) RAM_FUNCTION;

#endif /* USE_KEYSCAN_DLPS */

/******************************************************************************/
/******************** [SPI DLPS] **********************************************/
/******************************************************************************/
/******************************************************************************/
#if USE_SPI0_DLPS || USE_SPI1_DLPS || USE_SPI0_SLAVE_DLPS
#include "rtl_spi.h"

#if USE_SPI0_DLPS
volatile SPIStoreReg_Typedef SPI0_StoreReg;
#endif
#if USE_SPI1_DLPS
volatile SPIStoreReg_Typedef SPI1_StoreReg;
#endif
#if USE_SPI0_SLAVE_DLPS
volatile SPIStoreReg_Typedef SPI0_SLAVE_StoreReg;
#endif

extern void SPI_DLPSEnter(void *PeriReg, void *StoreBuf) RAM_FUNCTION;
extern void SPI_DLPSExit(void *PeriReg, void *StoreBuf) RAM_FUNCTION;

#endif /* USE_SPI0_DLPS || USE_SPI1_DLPS || USE_SPI0_SLAVE_DLPS */

/******************************************************************************/
/******************** [Timer & PWM DLPS] **************************************/
/******************************************************************************/
/******************************************************************************/
#if USE_TIM_DLPS
#include "rtl_tim.h"

volatile TIMStoreReg_Typedef TIM_StoreReg;

extern void TIM_DLPSEnter(void *PeriReg, void *StoreBuf) RAM_FUNCTION;
extern void TIM_DLPSExit(void *PeriReg, void *StoreBuf) RAM_FUNCTION;

#endif  /* USE_TIM_DLPS */

/******************************************************************************/
/****************************** [Enhance Timer & PWM DLPS] ********************/
/******************************************************************************/
/******************************************************************************/
#if USE_ENHTIM_DLPS
#include "rtl_enh_tim.h"

volatile ENHTIMStoreReg_Typedef ENHTIM_StoreReg;

extern void ENHTIM_DLPSEnter(void *PeriReg, void *StoreBuf) RAM_FUNCTION;
extern void ENHTIM_DLPSExit(void *PeriReg, void *StoreBuf) RAM_FUNCTION;

#endif  /* USE_ENHTIM_DLPS */

/******************************************************************************/
/****************************** [SPI3WIRE DLPS] ********************/
/******************************************************************************/
/******************************************************************************/
#if USE_SPI3W_DLPS
#include "rtl_spi3w.h"

volatile SPI3WStoreReg_Typedef SPI3W_StoreReg;

extern void SPI3W_DLPSEnter(void *PeriReg, void *StoreBuf) RAM_FUNCTION;
extern void SPI3W_DLPSExit(void *PeriReg, void *StoreBuf) RAM_FUNCTION;

#endif  /* USE_SPI3W_DLPS */

/******************************************************************************/
/******************** [UART DLPS] *********************************************/
/******************************************************************************/
/******************************************************************************/
#if USE_UART0_DLPS || USE_UART1_DLPS || USE_UART2_DLPS || USE_UART3_DLPS || USE_UART4_DLPS || USE_UART5_DLPS
#include "rtl_uart.h"

#if USE_UART0_DLPS
volatile UARTStoreReg_Typedef UART0_StoreReg;
#endif
#if USE_UART1_DLPS
volatile UARTStoreReg_Typedef UART1_StoreReg;
#endif
#if USE_UART2_DLPS
volatile UARTStoreReg_Typedef UART2_StoreReg;
#endif
#if USE_UART3_DLPS
volatile UARTStoreReg_Typedef UART3_StoreReg;
#endif
#if USE_UART4_DLPS
volatile UARTStoreReg_Typedef UART4_StoreReg;
#endif
#if USE_UART5_DLPS
volatile UARTStoreReg_Typedef UART5_StoreReg;
#endif

extern void UART_DLPSEnter(void *PeriReg, void *StoreBuf) RAM_FUNCTION;
extern void UART_DLPSExit(void *PeriReg, void *StoreBuf) RAM_FUNCTION;

#endif /* USE_UART0_DLPS || USE_UART1_DLPS || USE_UART2_DLPS || USE_UART3_DLPS || USE_UART4_DLPS || USE_UART5_DLPS */

/*============================================================================*
 *                         Platform DLPS
 *============================================================================*/

/******************************************************************************/
/******************** [USER Enter DLPS CALLBACK FUNC] *************************/
/******************************************************************************/
/******************************************************************************/
#if USE_USER_DEFINE_DLPS_ENTER_CB

DLPS_IO_EnterDlpsCB User_IO_EnterDlpsCB = NULL;

#endif /* USE_USER_DEFINE_DLPS_ENTER_CB */

/******************************************************************************/
/******************** [USER Exit DLPS CALLBACK FUNC] **************************/
/******************************************************************************/
/******************************************************************************/
#if USE_USER_DEFINE_DLPS_EXIT_CB

DLPS_IO_ExitDlpsCB User_IO_ExitDlpsCB = NULL;

#endif /* USE_USER_DEFINE_DLPS_EXIT_CB */


#if USE_IO_DRIVER_DLPS

/******************************************************************************/
/******************** [CPU & NVIC DLPS] ***************************************/
/******************************************************************************/
/******************************************************************************/

__STATIC_INLINE void CPU_DLPS_Enter(void);
__STATIC_INLINE void CPU_DLPS_Exit(void);

volatile uint32_t
CPU_StoreReg[6];         /*  This array should be placed in RAM ON/Buffer ON.    */
volatile uint8_t CPU_StoreReg_IPR[96];
volatile uint32_t Peripheral_StoreReg[2];

/**
  * @brief  CPU enter dlps callback function(Save CPU register values when system enter DLPS)
  * @param  None
  * @retval None
  */
__STATIC_INLINE void CPU_DLPS_Enter(void) RAM_FUNCTION;
void CPU_DLPS_Enter(void)
{
    //NVIC store
    uint32_t i;

    CPU_StoreReg[0] = NVIC->ISER[0];
    CPU_StoreReg[1] = NVIC->ISER[1];
    CPU_StoreReg[2] = NVIC->ISER[2];

    CPU_StoreReg[3] = NVIC->ISPR[0];
    CPU_StoreReg[4] = NVIC->ISPR[1];
    CPU_StoreReg[5] = NVIC->ISPR[2];

    //     skip System_IRQn, WDG_IRQn, RXI300_IRQn, RXI300_SEC_IRQn,
    //     Zigbee_IRQn which are handled in rom
    const uint8_t *IPR_pt = (const uint8_t *)NVIC->IPR;
    for (i = 5; i < 96; ++i)
    {
        CPU_StoreReg_IPR[i] = IPR_pt[i];
    }

    // peripheral reg store
    Peripheral_StoreReg[0] = SoC_VENDOR->u_008.REG_LOW_PRI_INT_MODE;
    Peripheral_StoreReg[1] = SoC_VENDOR->u_00C.REG_LOW_PRI_INT_EN;

    return;
}

/**
  * @brief  CPU exit dlps callback function(Resume CPU register values when system exit DLPS)
  * @param  None
  * @retval None
  */
__STATIC_INLINE  void CPU_DLPS_Exit(void) RAM_FUNCTION;
void CPU_DLPS_Exit(void)
{
    // peripheral reg restore
    SoC_VENDOR->u_008.REG_LOW_PRI_INT_MODE = Peripheral_StoreReg[0];
    SoC_VENDOR->u_00C.REG_LOW_PRI_INT_EN = Peripheral_StoreReg[1];

    //NVIC restore
    uint32_t i;

    //Don't restore NVIC pending register, but report warning
    //NVIC->ISPR[0] = CPU_StoreReg[3];
    //NVIC->ISPR[1] = CPU_StoreReg[4];
    //NVIC->ISPR[2] = CPU_StoreReg[5];

    if ((CPU_StoreReg[0] & CPU_StoreReg[3]) || (CPU_StoreReg[1] & CPU_StoreReg[4]) ||
        (CPU_StoreReg[2] & CPU_StoreReg[5]))
    {
        /* During enter and exit dlps, system will disable all interrupts. If any interrupt occurs during this period, this log will be printed.
        Every bit of pending register corresponds to an interrupt. Please refer to IRQn_Type from System_IRQn to PF_RTC_IRQn.
        For example:  "miss interrupt: pending register: 0x100, 0x0 , 0x0"
        It means that RTC interrupt occur during dlps store and restore flow. But because all interrupts are masked, these interrupts are pending.
        */
        OS_PRINT_WARN3("miss interrupt: pending register: 0x%x, 0x%x, 0x%x", CPU_StoreReg[3],
                       CPU_StoreReg[4], CPU_StoreReg[5]);
    }

    //     skip System_IRQn, WDG_IRQn, RXI300_IRQn, RXI300_SEC_IRQn,
    //     Zigbee_IRQn which are handled in rom
    uint8_t *IPR_pt = (uint8_t *)NVIC->IPR;
    for (i = 5; i < 96; ++i)
    {
        IPR_pt[i] = CPU_StoreReg_IPR[i];
    }

    NVIC->ISER[0] = CPU_StoreReg[0];
    NVIC->ISER[1] = CPU_StoreReg[1];
    NVIC->ISER[2] = CPU_StoreReg[2];

    return;
}


/******************************************************************************/
/******************** [Enter & Exit DLPS CALLBACK FUNC] ***********************/
/******************************************************************************/
/******************************************************************************/

/**
  * @brief  IO enter dlps callback function
  * @param  None
  * @retval None
  */
void DLPS_IO_EnterDlpsCb(void) RAM_FUNCTION;
void DLPS_IO_EnterDlpsCb(void)
{
    /* low stack do it instead */
    Pad_ClearAllWakeupINT();
    System_WakeupDebounceClear(0);

    NVIC_DisableIRQ(System_IRQn);
    CPU_DLPS_Enter();

    Pinmux_DLPSEnter(PINMUX, (void *)&Pinmux_StoreReg);

#if USE_USER_DEFINE_DLPS_ENTER_CB
    if (User_IO_EnterDlpsCB)
    {
        User_IO_EnterDlpsCB();
    }
#endif

#if USE_ADC_DLPS
    ADC_DLPSEnter(ADC, (void *)&ADC_StoreReg);
#endif

#if USE_GPIOA_DLPS
    GPIO_DLPSEnter(GPIOA, (void *)&GPIOA_StoreReg);
#endif

#if USE_GPIOB_DLPS
    GPIO_DLPSEnter(GPIOB, (void *)&GPIOB_StoreReg);
#endif

#if USE_I2C0_DLPS
    I2C_DLPSEnter(I2C0, (void *)&I2C0_StoreReg);
#endif

#if USE_I2C1_DLPS
    I2C_DLPSEnter(I2C1, (void *)&I2C1_StoreReg);
#endif

#if USE_I2C2_DLPS
    I2C_DLPSEnter(I2C2, (void *)&I2C2_StoreReg);
#endif

#if USE_I2C3_DLPS
    I2C_DLPSEnter(I2C3, (void *)&I2C3_StoreReg);
#endif

#if USE_IR_DLPS
    IR_DLPSEnter(IR, (void *)&IR_StoreReg);
#endif

#if USE_KEYSCAN_DLPS
    KEYSCAN_DLPSEnter(KEYSCAN, (void *)&KeyScan_StoreReg);
#endif

#if USE_SPI0_DLPS
    SPI_DLPSEnter(SPI0, (void *)&SPI0_StoreReg);
#endif

#if USE_SPI1_DLPS
    SPI_DLPSEnter(SPI1, (void *)&SPI1_StoreReg);
#endif

#if USE_SPI0_SLAVE_DLPS
    SPI_DLPSEnter(SPI0_SLAVE, (void *)&SPI0_SLAVE_StoreReg);
#endif

#if USE_TIM_DLPS
    TIM_DLPSEnter(TIM0, (void *)&TIM_StoreReg);
#endif

#if USE_ENHTIM_DLPS
    ENHTIM_DLPSEnter(ENH_TIM0, (void *)&ENHTIM_StoreReg);
#endif

#if USE_SPI3W_DLPS
    SPI3W_DLPSEnter(SPI3W, (void *)&SPI3W_StoreReg);
#endif

#if USE_UART0_DLPS
    UART_DLPSEnter(UART0, (void *)&UART0_StoreReg);
#endif

#if USE_UART1_DLPS
    UART_DLPSEnter(UART1, (void *)&UART1_StoreReg);
#endif

#if USE_UART2_DLPS
    UART_DLPSEnter(UART2, (void *)&UART2_StoreReg);
#endif

#if USE_UART3_DLPS
    UART_DLPSEnter(UART3, (void *)&UART3_StoreReg);
#endif

#if USE_UART4_DLPS
    UART_DLPSEnter(UART4, (void *)&UART4_StoreReg);
#endif

#if USE_UART5_DLPS
    UART_DLPSEnter(UART5, (void *)&UART5_StoreReg);
#endif

#if USE_TRNG_DLPS
    extern void deinit_true_random_generator(bool enter_lpm);
    deinit_true_random_generator(true);
#endif

#if USE_PSRAM
#if USE_PSRAM_DEEP_POWER_HALF_SLEEP_MODE
    fmc_psram_wb_set_partial_refresh(FMC_FLASH_NOR_IDX1, FMC_PSRAM_WB_REFRESH_TOP_1_2);
    fmc_psram_enter_lpm(FMC_FLASH_NOR_IDX1, FMC_PSRAM_LPM_HALF_SLEEP_MODE);
#else
    fmc_psram_enter_lpm(FMC_FLASH_NOR_IDX1, FMC_PSRAM_LPM_DEEP_POWER_DOWN_MODE);
#endif
#endif
    fmc_pad_ctrl_in_lps_mode(FMC_FLASH_NOR_IDX1, true);

    if (power_mode_get() == POWER_POWERDOWN_MODE)
    {
        AON_NS_REG0X_APP_TYPE aon_0x1ae0 = {.d32 = AON_REG_READ(AON_NS_REG0X_APP)};
        aon_0x1ae0.reset_reason = RESET_REASON_POWER_DOWN;
        AON_REG_WRITE(AON_NS_REG0X_APP, aon_0x1ae0.d32);
        DBG_DIRECT("Power down: 0x%x", AON_REG_READ(AON_NS_REG0X_APP));
    }
    else if (power_mode_get() == POWER_DLPS_MODE)
    {
        AON_NS_REG0X_APP_TYPE aon_0x1ae0 = {.d32 = AON_REG_READ(AON_NS_REG0X_APP)};
        aon_0x1ae0.reset_reason = RESET_REASON_DLPS;
        AON_REG_WRITE(AON_NS_REG0X_APP, aon_0x1ae0.d32);
    }

}

/**
  * @brief  IO exit dlps callback function.
  * @param  None
  * @retval None
  */
void DLPS_IO_ExitDlpsCb(void) RAM_FUNCTION;
void DLPS_IO_ExitDlpsCb(void)
{
    if (power_mode_get() == POWER_DLPS_MODE)
    {
        AON_NS_REG0X_APP_TYPE aon_0x1ae0 = {.d32 = AON_REG_READ(AON_NS_REG0X_APP)};
        aon_0x1ae0.reset_reason = RESET_REASON_HW;
        AON_REG_WRITE(AON_NS_REG0X_APP, aon_0x1ae0.d32);
    }

    Pinmux_DLPSExit(PINMUX, (void *)&Pinmux_StoreReg);

#if USE_ADC_DLPS
    ADC_DLPSExit(ADC, (void *)&ADC_StoreReg);
#endif

#if USE_GPIOA_DLPS
    GPIO_DLPSExit(GPIOA, (void *)&GPIOA_StoreReg);
#endif

#if USE_GPIOB_DLPS
    GPIO_DLPSExit(GPIOB, (void *)&GPIOB_StoreReg);
#endif

#if USE_I2C0_DLPS
    I2C_DLPSExit(I2C0, (void *)&I2C0_StoreReg);
#endif

#if USE_I2C1_DLPS
    I2C_DLPSExit(I2C1, (void *)&I2C1_StoreReg);
#endif

#if USE_I2C2_DLPS
    I2C_DLPSExit(I2C2, (void *)&I2C2_StoreReg);
#endif

#if USE_I2C3_DLPS
    I2C_DLPSExit(I2C3, (void *)&I2C3_StoreReg);
#endif

#if USE_IR_DLPS
    IR_DLPSExit(IR, (void *)&IR_StoreReg);
#endif

#if USE_KEYSCAN_DLPS
    KEYSCAN_DLPSExit(KEYSCAN, (void *)&KeyScan_StoreReg);
#endif

#if USE_SPI0_DLPS
    SPI_DLPSExit(SPI0, (void *)&SPI0_StoreReg);
#endif

#if USE_SPI1_DLPS
    SPI_DLPSExit(SPI1, (void *)&SPI1_StoreReg);
#endif

#if USE_SPI0_SLAVE_DLPS
    SPI_DLPSExit(SPI0_SLAVE, (void *)&SPI0_SLAVE_StoreReg);
#endif

#if USE_TIM_DLPS
    TIM_DLPSExit(TIM0, (void *)&TIM_StoreReg);
#endif

#if USE_ENHTIM_DLPS
    ENHTIM_DLPSExit(ENH_TIM0, (void *)&ENHTIM_StoreReg);
#endif

#if USE_SPI3W_DLPS
    SPI3W_DLPSExit(SPI3W, (void *)&SPI3W_StoreReg);
#endif

#if USE_UART0_DLPS
    UART_DLPSExit(UART0, (void *)&UART0_StoreReg);
#endif

#if USE_UART1_DLPS
    UART_DLPSExit(UART1, (void *)&UART1_StoreReg);
#endif

#if USE_UART2_DLPS
    UART_DLPSExit(UART2, (void *)&UART2_StoreReg);
#endif

#if USE_UART3_DLPS
    UART_DLPSExit(UART3, (void *)&UART3_StoreReg);
#endif

#if USE_UART4_DLPS
    UART_DLPSExit(UART4, (void *)&UART4_StoreReg);
#endif

#if USE_UART5_DLPS
    UART_DLPSExit(UART5, (void *)&UART5_StoreReg);
#endif

#if USE_PSRAM
    fmc_pad_ctrl_in_lps_mode(FMC_FLASH_NOR_IDX1, false);
#if USE_PSRAM_DEEP_POWER_HALF_SLEEP_MODE
    fmc_psram_exit_lpm(FMC_FLASH_NOR_IDX1, FMC_PSRAM_LPM_HALF_SLEEP_MODE);
#else
    fmc_psram_exit_lpm(FMC_FLASH_NOR_IDX1, FMC_PSRAM_LPM_DEEP_POWER_DOWN_MODE);
#endif
#endif

#if USE_PKE_DLPS
    extern void hw_pke_clock(bool enable);
    hw_pke_clock(true);
    extern void hw_pke_init(bool byte_swap_en, bool word_swap_en, uint32_t word_swap_base);
    hw_pke_init(false, false, 0);
#endif

#if USE_SHA256_DLPS
    extern void hw_sha256_init(void);
    hw_sha256_init();
#endif

#if USE_USER_DEFINE_DLPS_EXIT_CB
    if (User_IO_ExitDlpsCB)
    {
        User_IO_ExitDlpsCB();
    }
#endif

#if (FEATURE_TRUSTZONE_ENABLE == 1)
    /*target IRQs to non-secure*/
    setup_non_secure_nvic();
#endif

    NVIC_InitTypeDef nvic_init_struct = {0};
    nvic_init_struct.NVIC_IRQChannel         = System_IRQn;
    nvic_init_struct.NVIC_IRQChannelCmd      = (FunctionalState)ENABLE;
    nvic_init_struct.NVIC_IRQChannelPriority = 3;
    NVIC_Init(&nvic_init_struct); //Enable SYSTEM_ON Interrupt

    CPU_DLPS_Exit();
}

/**
  * @brief  register IO DLPS callback function
  * @param  None
  * @retval None
  */
void DLPS_IORegister(void)
{
    power_stage_cb_register(DLPS_IO_EnterDlpsCb, POWER_STAGE_STORE);
    power_stage_cb_register(DLPS_IO_ExitDlpsCb, POWER_STAGE_RESTORE);
    power_check_cb_register(power_clock_check_cb);

    return;
}

#endif /* USE_IO_DRIVER_DLPS */




