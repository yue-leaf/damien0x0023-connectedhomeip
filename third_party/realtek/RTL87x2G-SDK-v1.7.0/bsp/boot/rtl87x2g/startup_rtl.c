/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include <string.h>
#include "mem_config.h"
#include "rtl876x.h"
#include "trace.h"
#include "utils.h"
#include "vector_table.h"

#define IMG_HEADER_SECTION      __attribute__((section(".image_entry")))


extern bool system_init(void);

bool Reset_Handler(void);
void Default_Handler(void);


#if defined (__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050)  || defined ( __GNUC__ )
//void Reset_Handler                (void) __attribute__ ((weak, alias("Default_Handler")));
void HardFault_Handler(void) __attribute__((weak, alias("Default_Handler")));
void NMI_Handler(void) __attribute__((weak, alias("Default_Handler")));
void MemManage_Handler(void) __attribute__((weak, alias("Default_Handler")));
void BusFault_Handler(void) __attribute__((weak, alias("Default_Handler")));
void UsageFault_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SVC_Handler(void) __attribute__((weak, alias("Default_Handler")));
void PendSV_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SysTick_Handler(void) __attribute__((weak, alias("Default_Handler")));

void System_Handler(void) __attribute__((weak, alias("Default_Handler")));
void WDT_Handler(void) __attribute__((weak, alias("Default_Handler")));
void RXI300_Handler(void) __attribute__((weak, alias("Default_Handler")));
void RXI300_SEC_Handler(void) __attribute__((weak, alias("Default_Handler")));
void Zigbee_Handler(void) __attribute__((weak, alias("Default_Handler")));
void BTMAC_Handler(void) __attribute__((weak, alias("Default_Handler")));
void Peripheral_Handler(void) __attribute__((weak, alias("Default_Handler")));
void Platform_RTC_Handler(void) __attribute__((weak, alias("Default_Handler")));
void RTC_Handler(void) __attribute__((weak, alias("Default_Handler")));
void GPIO_A0_Handler(void) __attribute__((weak, alias("Default_Handler")));
void GPIO_A1_Handler(void) __attribute__((weak, alias("Default_Handler")));
void GPIO_A_2_7_Handler(void) __attribute__((weak, alias("Default_Handler")));
void GPIO_A_8_15_Handler(void) __attribute__((weak, alias("Default_Handler")));
void GPIO_A_16_23_Handler(void) __attribute__((weak, alias("Default_Handler")));
void GPIO_A_24_31_Handler(void) __attribute__((weak, alias("Default_Handler")));
void GPIO_B_0_7_Handler(void) __attribute__((weak, alias("Default_Handler")));
void GPIO_B_8_15_Handler(void) __attribute__((weak, alias("Default_Handler")));
void GPIO_B_16_23_Handler(void) __attribute__((weak, alias("Default_Handler")));
void GPIO_B_24_31_Handler(void) __attribute__((weak, alias("Default_Handler")));
void GDMA0_Channel0_Handler(void) __attribute__((weak, alias("Default_Handler")));
void GDMA0_Channel1_Handler(void) __attribute__((weak, alias("Default_Handler")));
void GDMA0_Channel2_Handler(void) __attribute__((weak, alias("Default_Handler")));
void GDMA0_Channel3_Handler(void) __attribute__((weak, alias("Default_Handler")));
void GDMA0_Channel4_Handler(void) __attribute__((weak, alias("Default_Handler")));
void GDMA0_Channel5_Handler(void) __attribute__((weak, alias("Default_Handler")));
void GDMA0_Channel6_Handler(void) __attribute__((weak, alias("Default_Handler")));
void GDMA0_Channel7_Handler(void) __attribute__((weak, alias("Default_Handler")));
void GDMA0_Channel8_Handler(void) __attribute__((weak, alias("Default_Handler")));
void GDMA0_Channel9_Handler(void) __attribute__((weak, alias("Default_Handler")));
void PPE_Handler(void) __attribute__((weak, alias("Default_Handler")));
void I2C0_Handler(void) __attribute__((weak, alias("Default_Handler")));
void I2C1_Handler(void) __attribute__((weak, alias("Default_Handler")));
void I2C2_Handler(void) __attribute__((weak, alias("Default_Handler")));
void I2C3_Handler(void) __attribute__((weak, alias("Default_Handler")));
void UART0_Handler(void) __attribute__((weak, alias("Default_Handler")));
void UART1_Handler(void) __attribute__((weak, alias("Default_Handler")));
void UART2_Handler(void) __attribute__((weak, alias("Default_Handler")));
void UART3_Handler(void) __attribute__((weak, alias("Default_Handler")));
void UART4_Handler(void) __attribute__((weak, alias("Default_Handler")));
void UART5_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SPI3W_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SPI0_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SPI1_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SPI_Slave_Handler(void) __attribute__((weak, alias("Default_Handler")));
void Timer0_Handler(void) __attribute__((weak, alias("Default_Handler")));
void Timer1_Handler(void) __attribute__((weak, alias("Default_Handler")));
void Timer2_Handler(void) __attribute__((weak, alias("Default_Handler")));
void Timer3_Handler(void) __attribute__((weak, alias("Default_Handler")));
void Timer4_Handler(void) __attribute__((weak, alias("Default_Handler")));
void Timer5_Handler(void) __attribute__((weak, alias("Default_Handler")));
void Timer6_Handler(void) __attribute__((weak, alias("Default_Handler")));
void Timer7_Handler(void) __attribute__((weak, alias("Default_Handler")));
void Enhanced_Timer0_Handler(void) __attribute__((weak, alias("Default_Handler")));
void Enhanced_Timer1_Handler(void) __attribute__((weak, alias("Default_Handler")));
void Enhanced_Timer2_Handler(void) __attribute__((weak, alias("Default_Handler")));
void Enhanced_Timer3_Handler(void) __attribute__((weak, alias("Default_Handler")));
void HRADC_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SAR_ADC_Handler(void) __attribute__((weak, alias("Default_Handler")));
void KEYSCAN_Handler(void) __attribute__((weak, alias("Default_Handler")));
void AON_QDEC_Handler(void) __attribute__((weak, alias("Default_Handler")));
void IR_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SDHC_Handler(void) __attribute__((weak, alias("Default_Handler")));
void ISO7816_Handler(void) __attribute__((weak, alias("Default_Handler")));
void Display_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SPORT0_RX_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SPORT0_TX_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SPORT1_RX_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SPORT1_TX_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SHA256_Handler(void) __attribute__((weak, alias("Default_Handler")));
void Public_Key_Engine_Handler(void) __attribute__((weak, alias("Default_Handler")));
void Flash_SEC_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SegCom_CTL_Handler(void) __attribute__((weak, alias("Default_Handler")));
void CAN_Handler(void) __attribute__((weak, alias("Default_Handler")));
void ETH_Handler(void) __attribute__((weak, alias("Default_Handler")));
void IDU_Handler(void) __attribute__((weak, alias("Default_Handler")));
void Slave_Port_Monitor_Handler(void) __attribute__((weak, alias("Default_Handler")));
void PTA_Mailbox_Handler(void) __attribute__((weak, alias("Default_Handler")));
void USB_Handler(void) __attribute__((weak, alias("Default_Handler")));
void USB_Sof_Handler(void) __attribute__((weak, alias("Default_Handler")));
void USB_Suspend_N_Handler(void) __attribute__((weak, alias("Default_Handler")));
void USB_Endp_Multi_Proc_Handler(void) __attribute__((weak, alias("Default_Handler")));
void USB_IN_EP_0_Handler(void) __attribute__((weak, alias("Default_Handler")));
void USB_IN_EP_1_Handler(void) __attribute__((weak, alias("Default_Handler")));
void USB_IN_EP_2_Handler(void) __attribute__((weak, alias("Default_Handler")));
void USB_IN_EP_3_Handler(void) __attribute__((weak, alias("Default_Handler")));
void USB_IN_EP_4_Handler(void) __attribute__((weak, alias("Default_Handler")));
void USB_IN_EP_5_Handler(void) __attribute__((weak, alias("Default_Handler")));
void USB_OUT_EP_0_Handler(void) __attribute__((weak, alias("Default_Handler")));
void USB_OUT_EP_1_Handler(void) __attribute__((weak, alias("Default_Handler")));
void USB_OUT_EP_2_Handler(void) __attribute__((weak, alias("Default_Handler")));
void USB_OUT_EP_3_Handler(void) __attribute__((weak, alias("Default_Handler")));
void USB_OUT_EP_4_Handler(void) __attribute__((weak, alias("Default_Handler")));
void USB_OUT_EP_5_Handler(void) __attribute__((weak, alias("Default_Handler")));
void TMETER_Handler(void) __attribute__((weak, alias("Default_Handler")));
void PF_RTC_Handler(void) __attribute__((weak, alias("Default_Handler")));
void BTMAC_WRAP_AROUND_Handler(void) __attribute__((weak, alias("Default_Handler")));
void Timer_A0_A1_Handler(void) __attribute__((weak, alias("Default_Handler")));
void BT_Bluewiz_Handler(void) __attribute__((weak, alias("Default_Handler")));
void BT_BZ_DMA_Handler(void) __attribute__((weak, alias("Default_Handler")));
void Proprietary_protocol_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SPIC0_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SPIC1_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SPIC2_Handler(void) __attribute__((weak, alias("Default_Handler")));
void TRNG_Handler(void) __attribute__((weak, alias("Default_Handler")));
void LPCOMP_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SPI_PHY1_Handler(void) __attribute__((weak, alias("Default_Handler")));
void GPIOA2_Handler(void) __attribute__((weak, alias("Default_Handler")));
void GPIOA3_Handler(void) __attribute__((weak, alias("Default_Handler")));
void GPIOA4_Handler(void) __attribute__((weak, alias("Default_Handler")));
void GPIOA5_Handler(void) __attribute__((weak, alias("Default_Handler")));
void GPIOA6_Handler(void) __attribute__((weak, alias("Default_Handler")));
void GPIOA7_Handler(void) __attribute__((weak, alias("Default_Handler")));
void GPIOA8_Handler(void) __attribute__((weak, alias("Default_Handler")));
void GPIOA9_Handler(void) __attribute__((weak, alias("Default_Handler")));
void GPIOA10_Handler(void) __attribute__((weak, alias("Default_Handler")));
void GPIOA11_Handler(void) __attribute__((weak, alias("Default_Handler")));
void GPIOA12_Handler(void) __attribute__((weak, alias("Default_Handler")));
void GPIOA13_Handler(void) __attribute__((weak, alias("Default_Handler")));
void GPIOA14_Handler(void) __attribute__((weak, alias("Default_Handler")));
void GPIOA15_Handler(void) __attribute__((weak, alias("Default_Handler")));
void GPIOA16_Handler(void) __attribute__((weak, alias("Default_Handler")));
void GPIOA17_Handler(void) __attribute__((weak, alias("Default_Handler")));
void GPIOA18_Handler(void) __attribute__((weak, alias("Default_Handler")));
void GPIOA19_Handler(void) __attribute__((weak, alias("Default_Handler")));
void GPIOA20_Handler(void) __attribute__((weak, alias("Default_Handler")));
void GPIOA21_Handler(void) __attribute__((weak, alias("Default_Handler")));
void GPIOA22_Handler(void) __attribute__((weak, alias("Default_Handler")));
void GPIOA23_Handler(void) __attribute__((weak, alias("Default_Handler")));
void GPIOA24_Handler(void) __attribute__((weak, alias("Default_Handler")));
void GPIOA25_Handler(void) __attribute__((weak, alias("Default_Handler")));
void GPIOA26_Handler(void) __attribute__((weak, alias("Default_Handler")));
void GPIOA27_Handler(void) __attribute__((weak, alias("Default_Handler")));
void GPIOA28_Handler(void) __attribute__((weak, alias("Default_Handler")));
void GPIOA29_Handler(void) __attribute__((weak, alias("Default_Handler")));
void GPIOA30_Handler(void) __attribute__((weak, alias("Default_Handler")));
void GPIOA31_Handler(void) __attribute__((weak, alias("Default_Handler")));
void GPIOB0_Handler(void) __attribute__((weak, alias("Default_Handler")));
void GPIOB1_Handler(void) __attribute__((weak, alias("Default_Handler")));
void GPIOB2_Handler(void) __attribute__((weak, alias("Default_Handler")));
void GPIOB3_Handler(void) __attribute__((weak, alias("Default_Handler")));
void GPIOB4_Handler(void) __attribute__((weak, alias("Default_Handler")));
void GPIOB5_Handler(void) __attribute__((weak, alias("Default_Handler")));
void GPIOB6_Handler(void) __attribute__((weak, alias("Default_Handler")));
void GPIOB7_Handler(void) __attribute__((weak, alias("Default_Handler")));
void GPIOB8_Handler(void) __attribute__((weak, alias("Default_Handler")));
void GPIOB9_Handler(void) __attribute__((weak, alias("Default_Handler")));
void GPIOB10_Handler(void) __attribute__((weak, alias("Default_Handler")));
void GPIOB11_Handler(void) __attribute__((weak, alias("Default_Handler")));
void GPIOB12_Handler(void) __attribute__((weak, alias("Default_Handler")));
void GPIOB13_Handler(void) __attribute__((weak, alias("Default_Handler")));
void GPIOB14_Handler(void) __attribute__((weak, alias("Default_Handler")));
void GPIOB15_Handler(void) __attribute__((weak, alias("Default_Handler")));
void GPIOB16_Handler(void) __attribute__((weak, alias("Default_Handler")));
void GPIOB17_Handler(void) __attribute__((weak, alias("Default_Handler")));
void GPIOB18_Handler(void) __attribute__((weak, alias("Default_Handler")));
void GPIOB19_Handler(void) __attribute__((weak, alias("Default_Handler")));
void GPIOB20_Handler(void) __attribute__((weak, alias("Default_Handler")));
void GPIOB21_Handler(void) __attribute__((weak, alias("Default_Handler")));
void GPIOB22_Handler(void) __attribute__((weak, alias("Default_Handler")));
void GPIOB23_Handler(void) __attribute__((weak, alias("Default_Handler")));
void GPIOB24_Handler(void) __attribute__((weak, alias("Default_Handler")));
void GPIOB25_Handler(void) __attribute__((weak, alias("Default_Handler")));
void GPIOB26_Handler(void) __attribute__((weak, alias("Default_Handler")));
void GPIOB27_Handler(void) __attribute__((weak, alias("Default_Handler")));
void GPIOB28_Handler(void) __attribute__((weak, alias("Default_Handler")));
void GPIOB29_Handler(void) __attribute__((weak, alias("Default_Handler")));
void GPIOB30_Handler(void) __attribute__((weak, alias("Default_Handler")));
void GPIOB31_Handler(void) __attribute__((weak, alias("Default_Handler")));
#endif


__attribute__((used)) const IRQ_Fun RamVectorTable_app[] __attribute__((
                                                                           section(".vectors_table"))) =
{
#if (FEATURE_TRUSTZONE_ENABLE == 1)
    (IRQ_Fun)(NS_RAM_MAIN_STACK_START_ADDR + NS_RAM_MAIN_STACK_SIZE),
#else
    (IRQ_Fun)(S_RAM_MAIN_STACK_START_ADDR + S_RAM_MAIN_STACK_SIZE),
#endif
    (IRQ_Fun)Reset_Handler,
    NMI_Handler,                /* -14 NMI Handler */
    HardFault_Handler,          /* -13 Hard Fault Handler */
    MemManage_Handler,          /* -12 MPU Fault Handler */
    BusFault_Handler,           /* -11 Bus Fault Handler */
    UsageFault_Handler,         /* -10 Usage Fault Handler */
    0,                          /*     Reserved */
    0,                          /*     Reserved */
    0,                          /*     Reserved */
    0,                          /*     Reserved */
    SVC_Handler,                /*  -5 SVC Handler */
    0,
    0,
    PendSV_Handler,             /*  -2 PendSV Handler */
    SysTick_Handler,            /*  -1 SysTick Handler */

    System_Handler,
    WDT_Handler,
    RXI300_Handler,
    RXI300_SEC_Handler,
    Zigbee_Handler,
    BTMAC_Handler,
    Peripheral_Handler,
    Default_Handler,
    RTC_Handler,
    GPIO_A0_Handler,
    GPIO_A1_Handler,
    GPIO_A_2_7_Handler,
    GPIO_A_8_15_Handler,
    GPIO_A_16_23_Handler,
    GPIO_A_24_31_Handler,
    GPIO_B_0_7_Handler,
    GPIO_B_8_15_Handler,
    GPIO_B_16_23_Handler,
    GPIO_B_24_31_Handler,
    GDMA0_Channel0_Handler,
    GDMA0_Channel1_Handler,
    GDMA0_Channel2_Handler,
    GDMA0_Channel3_Handler,
    GDMA0_Channel4_Handler,
    GDMA0_Channel5_Handler,
    GDMA0_Channel6_Handler,
    GDMA0_Channel7_Handler,
    GDMA0_Channel8_Handler,
    GDMA0_Channel9_Handler,
    PPE_Handler,
    I2C0_Handler,
    I2C1_Handler,
    I2C2_Handler,
    I2C3_Handler,
    UART0_Handler,
    UART1_Handler,
    UART2_Handler,
    UART3_Handler,
    UART4_Handler,
    UART5_Handler,
    SPI3W_Handler,
    SPI0_Handler,
    SPI1_Handler,
    SPI_Slave_Handler,
    Timer0_Handler,
    Timer1_Handler,
    Timer2_Handler,
    Timer3_Handler,
    Timer4_Handler,
    Timer5_Handler,
    Timer6_Handler,
    Timer7_Handler,
    Enhanced_Timer0_Handler,
    Enhanced_Timer1_Handler,
    Enhanced_Timer2_Handler,
    Enhanced_Timer3_Handler,
    HRADC_Handler,
    SAR_ADC_Handler,
    KEYSCAN_Handler,
    AON_QDEC_Handler,
    IR_Handler,
    SDHC_Handler,
    ISO7816_Handler,
    Display_Handler,
    SPORT0_RX_Handler,
    SPORT0_TX_Handler,
    SPORT1_RX_Handler,
    SPORT1_TX_Handler,
    SHA256_Handler,
    Public_Key_Engine_Handler,
    Flash_SEC_Handler,
    SegCom_CTL_Handler,
    CAN_Handler,
    ETH_Handler,
    IDU_Handler,
    Slave_Port_Monitor_Handler,
    PTA_Mailbox_Handler,
    USB_Handler,
    USB_Suspend_N_Handler,
    USB_Endp_Multi_Proc_Handler,
    USB_IN_EP_0_Handler,
    USB_IN_EP_1_Handler,
    USB_IN_EP_2_Handler,
    USB_IN_EP_3_Handler,
    USB_IN_EP_4_Handler,
    USB_IN_EP_5_Handler,
    USB_OUT_EP_0_Handler,
    USB_OUT_EP_1_Handler,
    USB_OUT_EP_2_Handler,
    USB_OUT_EP_3_Handler,
    USB_OUT_EP_4_Handler,
    USB_OUT_EP_5_Handler,
    USB_Sof_Handler,
    TMETER_Handler,
    PF_RTC_Handler,
    BTMAC_WRAP_AROUND_Handler,

    /* second level interrupt (BTMAC_IRQn), not directly connect to NVIC */
    Timer_A0_A1_Handler,
    BT_Bluewiz_Handler,
    Default_Handler,
    BT_BZ_DMA_Handler,
    Proprietary_protocol_Handler,
    Default_Handler,

    /* second level interrupt (Peripheral_IRQn), not directly connect to NVIC */
    SPIC0_Handler,
    SPIC1_Handler,
    SPIC2_Handler,
    TRNG_Handler,
    LPCOMP_Handler,
    SPI_PHY1_Handler,
    Default_Handler,

    /* gpio sub interrupt */
    GPIOA2_Handler,
    GPIOA3_Handler,
    GPIOA4_Handler,
    GPIOA5_Handler,
    GPIOA6_Handler,
    GPIOA7_Handler,
    GPIOA8_Handler,
    GPIOA9_Handler,
    GPIOA10_Handler,
    GPIOA11_Handler,
    GPIOA12_Handler,
    GPIOA13_Handler,
    GPIOA14_Handler,
    GPIOA15_Handler,
    GPIOA16_Handler,
    GPIOA17_Handler,
    GPIOA18_Handler,
    GPIOA19_Handler,
    GPIOA20_Handler,
    GPIOA21_Handler,
    GPIOA22_Handler,
    GPIOA23_Handler,
    GPIOA24_Handler,
    GPIOA25_Handler,
    GPIOA26_Handler,
    GPIOA27_Handler,
    GPIOA28_Handler,
    GPIOA29_Handler,
    GPIOA30_Handler,
    GPIOA31_Handler,

    GPIOB0_Handler,
    GPIOB1_Handler,
    GPIOB2_Handler,
    GPIOB3_Handler,
    GPIOB4_Handler,
    GPIOB5_Handler,
    GPIOB6_Handler,
    GPIOB7_Handler,
    GPIOB8_Handler,
    GPIOB9_Handler,
    GPIOB10_Handler,
    GPIOB11_Handler,
    GPIOB12_Handler,
    GPIOB13_Handler,
    GPIOB14_Handler,
    GPIOB15_Handler,
    GPIOB16_Handler,
    GPIOB17_Handler,
    GPIOB18_Handler,
    GPIOB19_Handler,
    GPIOB20_Handler,
    GPIOB21_Handler,
    GPIOB22_Handler,
    GPIOB23_Handler,
    GPIOB24_Handler,
    GPIOB25_Handler,
    GPIOB26_Handler,
    GPIOB27_Handler,
    GPIOB28_Handler,
    GPIOB29_Handler,
    GPIOB30_Handler,
    GPIOB31_Handler,
};
/**  static check VECTOR_TABLE_ITEMS value must be equal to MAX_VECTORn but also sizeof(secure RamVectorTable)/4
 *   To save code size, non-secure ram vector table must occupy the same size as secure ram vector
 *   the interrupt handlers will be initialized by update vector API.
**/
PLATFORM_STATIC_ASSERT(VECTOR_TABLE_ITEMS == MAX_VECTORn, VECTOR_TABLE_ITEMS);
PLATFORM_STATIC_ASSERT((VECTOR_TABLE_ITEMS << 2) == sizeof(RamVectorTable_app), RamVectorTable_app);

__attribute__((naked)) void __user_setup_stackheap(void)
{
    __asm volatile(
        "BX      lr                        \n"
    );

}

bool Reset_Handler(void) IMG_HEADER_SECTION;
bool Reset_Handler(void)
{
    system_init();

    return true;
}

/*----------------------------------------------------------------------------
  Default Handler for Exceptions / Interrupts
 *----------------------------------------------------------------------------*/
void Default_Handler(void)
{
    DBG_DIRECT("Error! Please implement your ISR Handler for IRQ number %d!", __get_IPSR());
    while (1);
}
