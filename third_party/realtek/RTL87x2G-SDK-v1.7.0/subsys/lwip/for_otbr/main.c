/**
*********************************************************************************************************
*               Copyright(c) 2023, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file     main.c
* @brief
* @details
* @author   mandy
* @date     2023-4-25
* @version  v1.0
*********************************************************************************************************
*/

/*============================================================================*
 *                              Header Files
 *============================================================================*/
#include "board.h"
#include "trace.h"
#include "sys_arch.h"
#include "test.h"
#include "flash_nor_device.h"
#include "app_section.h"
#include "lwip/sys.h"
#include "os_sched.h"
#include "os_task.h"
#include "clock.h"

/*============================================================================*
 *                              Local Macros
 *============================================================================*/
#define APP_TASK_STACK_SIZE         512

/*============================================================================*
 *                              Local Variables
 *============================================================================*/
static TaskHandle_t app_task_handle = NULL;

/*============================================================================*
 *                              Global Variables
 *============================================================================*/

/*============================================================================*
 *                              Functions Declaration
 *============================================================================*/

/*============================================================================*
 *                              Local Functions
 *============================================================================*/
/******************************************************************
 * @brief  init TCPIP and test code
 * @param  none
 * @return none
 */
static void app_main_task(void)
{
    TCPIP_Init();
    //test_init();

    taskENTER_CRITICAL();
    vTaskDelete(app_task_handle);
    taskEXIT_CRITICAL();
}

/******************************************************************
 * @brief  check cpu clk
 * @param  none
 * @return none
 */
void check_cpu_clk(void)
{
    uint32_t beg, end = 0;
    dwt_cyccnt_init();
    beg = read_cpu_counter();
    platform_delay_ms(1000);
    end = read_cpu_counter();

    DBG_DIRECT("CPU counter: beg %d, end %d, end - beg %d", beg, end, end - beg);
}
/*============================================================================*
*                              Global Functions
*============================================================================*/
/******************************************************************
 * @brief  main function
 * @param  none
 * @return none
 */
RAM_FUNCTION
int main(void)
{
    APP_PRINT_INFO0("[main] in main func");

    uint32_t actual_mhz;
    pm_cpu_freq_set(125, &actual_mhz);
    DBG_DIRECT("CPU actual %d(MHz)", actual_mhz);
    check_cpu_clk();

    fmc_flash_nor_clock_switch(FLASH_NOR_IDX_SPIC0, 160, &actual_mhz);
    DBG_DIRECT("SPIC0 actual %d(MHz)", actual_mhz);

    pm_spic1_freq_set(160, &actual_mhz);
    DBG_DIRECT("SPIC1 actual %d(MHz)", actual_mhz);

    FLASH_NOR_RET_TYPE ret = flash_nor_try_high_speed_mode(FLASH_NOR_IDX_SPIC0,
                                                           FLASH_NOR_4_BIT_MODE);
    APP_PRINT_INFO1("flash_nor_try_high_speed_mode ret = %d ", ret);

    extern uint32_t random_seed_value;
    srand(random_seed_value);

    app_task_handle = (TaskHandle_t)sys_thread_new("app", (lwip_thread_fn)app_main_task, NULL,
                                                   APP_TASK_STACK_SIZE, 1);

    if (NULL != app_task_handle)
    {
        vTaskStartScheduler();
    }
    else
    {
        return -1;
    }

    while (1);
}

/******************* (C) COPYRIGHT 2023 Realtek Semiconductor Corporation *****END OF FILE****/
