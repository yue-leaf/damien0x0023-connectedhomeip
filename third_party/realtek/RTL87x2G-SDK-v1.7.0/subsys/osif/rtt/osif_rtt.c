/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include <stdint.h>
#include <string.h>
#include "app_section.h"
#include "cmsis_compiler.h"
#include "rtthread.h"
#include "osif.h"
#include "osif_rtt.h"
#include "os_queue.h"
#include "os_msg.h"
#include "os_mem.h"
#include "os_sched.h"
#include "os_sync.h"
#include "os_timer.h"
#include "os_task.h"
#include "os_trace.h"
#include "dlps.h"
#include "os_pm.h"
#include "system_rtl876x.h"
#include "utils.h"
#include "trace.h"
#include "vector_table.h"
#include "rtl_nvic.h"
#include "mem_config.h"
#include "trace.h"
#include "os_cfg.h"
#include "platform_cfg.h"
#include "flash_nor_device.h"
#include "rthw.h"
#include "secure_init.h"

extern uint32_t SystemCoreClock;

#define DBG_ENABLE
#define DBG_TAG     "OSIF"
#define DBG_LVL     DBG_LOG
#include <rtdbg.h>

#define OSIF_TICK_PERIOD_MS        (1000 / RT_TICK_PER_SECOND)
#define OSIF_TICK_RATE_HZ          (RT_TICK_PER_SECOND)
#define OSIF_CPU_CLOCK_HZ         ( SystemCoreClock )
#define OSIF_SYSTICK_CLOCK_HZ      ((sys_init_cfg.systick_clk_src == SYSTICK_PROCESSOR_CLOCK) ? OSIF_CPU_CLOCK_HZ : sys_init_cfg.systick_ext_clk_freq )
#define portNVIC_SYSTICK_CLK_BIT   ( sys_init_cfg.systick_clk_src << 2UL )

uint32_t TopOfIdleTaskStack = 0;
uint32_t IdleTaskStackLim = 0;

static struct rt_memheap *data_on_heap = NULL;
static struct rt_memheap buffer_on_heap = {0};
struct rt_memheap ext_data_sram_heap = {0};

task_sem_item task_sem_array[TASK_SEM_ARRAY_NUMBER] = {0};

#ifdef RT_USING_TIMER_SOFT
extern struct rt_thread timer_thread;
void *sw_timer_task_handler = &timer_thread;
#endif

extern rt_err_t rt_trustzone_enter(rt_ubase_t module);
extern void rt_trustzone_init(void);
extern void rt_trustzone_context_load(rt_ubase_t context);

/**
 * @brief Constants required to manipulate the NVIC.
 */
#define portNVIC_SYSTICK_CTRL_REG             ( *( ( volatile uint32_t * ) 0xe000e010 ) )
#define portNVIC_SYSTICK_LOAD_REG             ( *( ( volatile uint32_t * ) 0xe000e014 ) )
#define portNVIC_SYSTICK_CURRENT_VALUE_REG    ( *( ( volatile uint32_t * ) 0xe000e018 ) )
#define portNVIC_SHPR3_REG                    ( *( ( volatile uint32_t * ) 0xe000ed20 ) )
#define portNVIC_SYSTICK_ENABLE_BIT           ( 1UL << 0UL )
#define portNVIC_SYSTICK_INT_BIT              ( 1UL << 1UL )
#define portNVIC_SYSTICK_COUNT_FLAG_BIT       ( 1UL << 16UL )
#define portMIN_INTERRUPT_PRIORITY            ( 255UL )
#define portNVIC_PENDSV_PRI                   ( portMIN_INTERRUPT_PRIORITY << 16UL )
#define portNVIC_SYSTICK_PRI                  ( portMIN_INTERRUPT_PRIORITY << 24UL )

/**
 * @brief Constants required to manipulate the FPU.
 */
#define portCPACR               ( ( volatile uint32_t * ) 0xe000ed88 )              /* Coprocessor Access Control Register. */
#define portCPACR_CP10_VALUE    ( 3UL )
#define portCPACR_CP11_VALUE    portCPACR_CP10_VALUE
#define portCPACR_CP10_POS      ( 20UL )
#define portCPACR_CP11_POS      ( 22UL )

#define portFPCCR               ( ( volatile uint32_t * ) 0xe000ef34 )              /* Floating Point Context Control Register. */
#define portFPCCR_ASPEN_POS     ( 31UL )
#define portFPCCR_ASPEN_MASK    ( 1UL << portFPCCR_ASPEN_POS )
#define portFPCCR_LSPEN_POS     ( 30UL )
#define portFPCCR_LSPEN_MASK    ( 1UL << portFPCCR_LSPEN_POS )
/*-----------------------------------------------------------*/

void SysTick_Handler(void)
{
    //DBG_DIRECT("function = %s, file = %s, line = %d",__FUNCTION__, __FILE_NAME__,__LINE__);
    /* enter interrupt */
    rt_interrupt_enter();

    rt_tick_increase();

    /* leave interrupt */
    rt_interrupt_leave();
}

#if ( configENABLE_FPU == 1 )
void prvSetupFPU(void)   /* PRIVILEGED_FUNCTION */
{
#if ( configENABLE_TRUSTZONE == 1 )
    {
        /* Enable non-secure access to the FPU. */
        SecureInit_EnableNSFPUAccess();
    }
#endif /* configENABLE_TRUSTZONE */

    /* CP10 = 11 ==> Full access to FPU i.e. both privileged and
     * unprivileged code should be able to access FPU. CP11 should be
     * programmed to the same value as CP10. */
    *(portCPACR) |= ((portCPACR_CP10_VALUE << portCPACR_CP10_POS) |
                     (portCPACR_CP11_VALUE << portCPACR_CP11_POS)
                    );

    /* ASPEN = 1 ==> Hardware should automatically preserve floating point
     * context on exception entry and restore on exception return.
     * LSPEN = 1 ==> Enable lazy context save of FP state. */
    *(portFPCCR) |= (portFPCCR_ASPEN_MASK | portFPCCR_LSPEN_MASK);
}
#endif /* configENABLE_FPU */

void vPortStartSchedulerInRestore(void)
{
    /* Make PendSV, CallSV and SysTick the same priority as the kernel. */
    portNVIC_SHPR3_REG |= portNVIC_PENDSV_PRI;
    portNVIC_SHPR3_REG |= portNVIC_SYSTICK_PRI;

#if ( configENABLE_MPU == 1 )
    {
        /* Setup the Memory Protection Unit (MPU). */
        prvSetupMPU();
    }
#endif /* configENABLE_MPU */

    /* Start the timer that generates the tick ISR. Interrupts are disabled
     * here already. */
    //vPortSetupTimerInterrupt();

    __DSB();
    __ISB();
}

T_OS_QUEUE lpm_excluded_handle[PLATFORM_PM_EXCLUDED_TYPE_MAX] =
{
    [0 ...(PLATFORM_PM_EXCLUDED_TYPE_MAX - 1)] = {
        .p_first = NULL,
        .p_last = NULL,
        .count = 0,
    },
};

/* Check if in task context (true), or isr context (false)                  */
/****************************************************************************/
__STATIC_INLINE bool os_task_context_check(void)
{
    return (__get_IPSR() == 0);
}


static uint32_t priority_convert_rtt2osif(uint32_t priority)
{
    if (priority == 0)
    {
        return 6;
    }
    else if (priority == 1)
    {
        return 5;
    }
    else if (priority == 2)
    {
        return 4;
    }
    else if (priority == 3)
    {
        return 3;
    }
    else if (priority == 4)
    {
        return 2;
    }
    else if (priority == 5)
    {
        return 1;
    }
    else if (priority == 31)
    {
        return 0;
    }
    else
    {
        RT_ASSERT(RT_NULL != RT_NULL);
        return 30;
    }
}
static uint32_t priority_convert_osif2rtt(uint32_t priority)
{
    if (priority == 6)
    {
        return 0;
    }
    else if (priority == 5)
    {
        return 1;
    }
    else if (priority == 4)
    {
        return 2;
    }
    else if (priority == 3)
    {
        return 3;
    }
    else if (priority == 2)
    {
        return 4;
    }
    else if (priority == 1)
    {
        return 5;
    }
    else if (priority == 0)
    {
        return 31;
    }
    else
    {
        RT_ASSERT(RT_NULL != RT_NULL);
        return 30;
    }
}

static void rt_hw_systick_init(void)
{
    /* Configure SysTick to interrupt at the requested rate. */
    SysTick->CTRL = 0UL;
    SysTick->LOAD = (OSIF_SYSTICK_CLOCK_HZ / OSIF_TICK_RATE_HZ) - 1UL;
    SysTick->VAL = 0UL;
    SysTick->CTRL = portNVIC_SYSTICK_CLK_BIT | portNVIC_SYSTICK_INT_BIT |
                    portNVIC_SYSTICK_ENABLE_BIT;

    NVIC_SetPriorityGrouping(2);

    NVIC_SetPriority(SysTick_IRQn, (1UL << __NVIC_PRIO_BITS) - 1UL);
    NVIC_EnableIRQ(SysTick_IRQn);
}

static void SystemClock_Config(void)
{

}

static
struct rt_memheap *rt_lookup_system_heap(void *begin_addr)
{
    struct rt_object *object;
    struct rt_list_node *node;
    struct rt_memheap *heap;
    struct rt_object_information *information;

    information = rt_object_get_information(RT_Object_Class_MemHeap);
    RT_ASSERT(information != RT_NULL);
    for (node  = information->object_list.next;
         node != &(information->object_list);
         node  = node->next)
    {
        object = rt_list_entry(node, struct rt_object, list);
        heap   = (struct rt_memheap *)object;

        RT_ASSERT(heap);
        RT_ASSERT(rt_object_get_type(&heap->parent) == RT_Object_Class_MemHeap);
        if (heap->start_addr == begin_addr)
        {
            return heap;
        }
    }
    return RT_NULL;
}

/****************************************************************************/
/* Delay current task in a given milliseconds                               */
/****************************************************************************/
void os_delay_RTT(uint32_t ms)
{
    rt_thread_delay((rt_tick_t)((ms + OSIF_TICK_PERIOD_MS - 1) / OSIF_TICK_PERIOD_MS));
}

/****************************************************************************/
/* Get system time in milliseconds                                          */
/****************************************************************************/
uint64_t os_sys_time_get_RTT(void)
{
    return rt_tick_get() * OSIF_TICK_PERIOD_MS;
}

/****************************************************************************/
/* Get system tick                                                          */
/****************************************************************************/
uint64_t os_sys_tick_get_RTT(void)
{
    return rt_tick_get();
}

/****************************************************************************/
/* Start os kernel scheduler                                                */
/****************************************************************************/
bool os_sched_start_RTT(void)
{
    prvSetupFPU();
//    rt_trustzone_init();
    rt_tick_set(0);

    rt_system_scheduler_start();

    return true;
}

/****************************************************************************/
/* Stop os kernel scheduler                                                 */
/****************************************************************************/
bool os_sched_stop_RTT(void)
{
    RT_ASSERT(RT_NULL != RT_NULL);
    rt_enter_critical();//TODO

    return true;
}

/****************************************************************************/
/* Suspend os kernel scheduler                                              */
/****************************************************************************/
bool os_sched_suspend_RTT(void)
{
    rt_enter_critical();

    return true;
}

/****************************************************************************/
/* Resume os kernel scheduler                                               */
/****************************************************************************/
bool os_sched_resume_RTT(void)
{
    rt_exit_critical();

    return true;
}

/****************************************************************************/
/*Get os kernel scheduler  state                                          */
/****************************************************************************/
bool os_sched_state_get_RTT(long *p_state)
{
    if (p_state == NULL)
    {
        return false;
    }

    if (rt_thread_self() == NULL)
    {
        *p_state = SCHEDULER_NOT_STARTED;
    }
    else if (rt_critical_level() > 0u)
    {
        *p_state = SCHEDULER_SUSPENDED;
    }
    else
    {
        *p_state = SCHEDULER_RUNNING;
    }

    return true;
}

/****************************************************************************/
/*Check os kernel scheduler is started                                      */
/****************************************************************************/
bool os_sched_is_start_RTT(void)
{
    if (rt_thread_self() == NULL)
    {
        return false;
    }
    else
    {
        return true;
    }
}

/****************************************************************************/
/*Update os kernel vector table                                            */
/****************************************************************************/
void os_vector_table_update_RTT(void)
{
    return;
}

/****************************************************************************/
/*os kernel init                                                            */
/****************************************************************************/
void osif_rtt_idle_hook(void)
{
    if (sys_init_cfg.use_ftl)
    {
        long state = 0;
        os_sched_state_get(&state);
        if (flash_nor_get_exist_nsc(FLASH_NOR_IDX_SPIC0) &&
            (SCHEDULER_SUSPENDED != state))
        {
            extern void (*flash_nor_remaining_work_check_in_idle)(void);
            if (flash_nor_remaining_work_check_in_idle)
            {
                flash_nor_remaining_work_check_in_idle();
            }

            extern void (*ftl_garbage_collect_check_in_idle)(void);
            if (ftl_garbage_collect_check_in_idle)
            {
                ftl_garbage_collect_check_in_idle();
            }
        }
    }

    extern void log_buffer_trigger_schedule_in_km4_idle_task(void);
    log_buffer_trigger_schedule_in_km4_idle_task();

    extern void (*power_manager_slave_inact_action_handler)(void);
    power_manager_slave_inact_action_handler();
}

void os_init_RTT(void)
{
    DBG_DIRECT("function = %s, line = %d", __FUNCTION__, __LINE__);
    rt_hw_interrupt_disable();

    extern void SVC_Handler(void);
    extern void PendSV_Handler(void);
    extern void SysTick_Handler(void);

    RamVectorTableUpdate(SVC_VECTORn, (IRQ_Fun)SVC_Handler);
    RamVectorTableUpdate(PendSV_VECTORn, (IRQ_Fun)PendSV_Handler);
    RamVectorTableUpdate(SysTick_VECTORn, (IRQ_Fun)SysTick_Handler);

    rt_hw_systick_init();

    /* Heap initialization */
#if defined(RT_USING_HEAP)
    DBG_DIRECT("function = %s, line = %d", __FUNCTION__, __LINE__);
    rt_system_heap_init((void *)(os_cfg.appDataAddr  + os_cfg.appDataSize),
                        (void *)(os_cfg.appDataAddr  + os_cfg.appDataSize + os_cfg.heapDataONSize));
    data_on_heap = rt_lookup_system_heap((void *)(os_cfg.appDataAddr  + os_cfg.appDataSize));


    DBG_DIRECT("function = %s, line = %d", __FUNCTION__, __LINE__);
    rt_memheap_init(&buffer_on_heap, "buffer_on", (void *)(BUFFER_ON_HEAP_ADDR),
                    os_cfg.heapBufferONSize);
    rt_memheap_init(&ext_data_sram_heap, "ext_data_ram",
                    (uint8_t *)(DATA_SRAM_ADDR + os_cfg.globalExtDataSRAMSize),
                    DATA_SRAM_SIZE - os_cfg.globalExtDataSRAMSize);
#endif
    DBG_DIRECT("function = %s, line = %d", __FUNCTION__, __LINE__);

    /* Pin driver initialization is open by default */
#ifdef RT_USING_PIN
    rt_hw_pin_init();
#endif

    /* USART driver initialization is open by default */
#ifdef RT_USING_SERIAL
    rt_hw_usart_init();
#endif

    /* ENHWTIMER driver initialization is open by default */
#ifdef BSP_USING_ENHTIMER0
    rt_hw_hwtimer_init();
#endif

    /* Set the shell console output device */
#ifdef RT_USING_CONSOLE
    rt_console_set_device(RT_CONSOLE_DEVICE_NAME);
#endif

    /* Board underlying hardware initialization */
#ifdef RT_USING_COMPONENTS_INIT
    rt_components_board_init();
#endif
    DBG_DIRECT("function = %s, file = %s, line = %d", __FUNCTION__, __FILE_NAME__, __LINE__);

    rt_show_version();

    /* timer system initialization */
    rt_system_timer_init();

    /* scheduler system initialization */
    rt_system_scheduler_init();

#ifdef RT_USING_SIGNALS
    /* signal system initialization */
    rt_system_signal_init();
#endif

    /* timer thread initialization */
    rt_system_timer_thread_init();
#ifdef RT_USING_TIMER_SOFT
    os_register_pm_excluded_handle(&sw_timer_task_handler, PLATFORM_PM_EXCLUDED_TASK);
#endif

    /* idle thread initialization */

    rt_thread_idle_init();
    rt_thread_idle_sethook(osif_rtt_idle_hook);

    rt_thread_t idle_handle = rt_thread_idle_gethandler();
    TopOfIdleTaskStack = (uint32_t)((rt_uint8_t *)(idle_handle->sp) + sizeof(struct stack_frame));
    IdleTaskStackLim = (uint32_t)idle_handle->stack_addr;

    return ;
}

/****************************************************************************/
/* Create os secure task stack so that the task can call NSC APIs           */
/****************************************************************************/
bool os_alloc_secure_ctx_RTT(uint32_t stack_size)
{
    rt_trustzone_enter(1);   // fixed 1KB size

    return true;
}

/****************************************************************************/
/* Create os level task routine                                             */
/****************************************************************************/
bool os_task_create_RTT(void **pp_handle, const char *p_name, void (*p_routine)(void *),
                        void *p_param, uint16_t stack_size, uint16_t priority)
{
    if (pp_handle == NULL)
    {
        return false;
    }

    uint16_t switch_priority = priority_convert_osif2rtt(priority);

    rt_thread_t thread = rt_thread_create((const char *)p_name, p_routine, p_param, stack_size,
                                          switch_priority, 2);
    *pp_handle = thread;

    if (*pp_handle != 0)
    {
        rt_thread_startup(thread);
        return true;
    }
    else
    {
        return false;
    }
}

/****************************************************************************/
/* Delete os level task routine                                             */
/****************************************************************************/
bool os_task_delete_RTT(void *p_handle)
{
    rt_base_t lock;
    lock = rt_hw_interrupt_disable();
    rt_thread_t thread = p_handle;
    if (thread == NULL)
    {
        thread = (void *)rt_thread_self();

        for (uint8_t i = 0; i < TASK_SEM_ARRAY_NUMBER; i++)
        {
            if (task_sem_array[i].task_handle == thread)
            {
                task_sem_array[i].task_handle = NULL;
                rt_sem_delete(task_sem_array[i].sem_handle);
                task_sem_array[i].sem_handle = NULL;
            }
        }
        rt_hw_interrupt_enable(lock);

        rt_thread_delete(thread);

        LOG_E("[EXIT TASK]task name = %s;\n", thread->name);
        return true;
    }
    else
    {
        for (uint8_t i = 0; i < TASK_SEM_ARRAY_NUMBER; i++)
        {
            if (task_sem_array[i].task_handle == thread)
            {
                task_sem_array[i].task_handle = NULL;
                rt_sem_delete(task_sem_array[i].sem_handle);
                task_sem_array[i].sem_handle = NULL;
            }
        }
        rt_hw_interrupt_enable(lock);

        rt_thread_delete(thread);

        LOG_E("[DELETE TASK]task name = %s;\n", thread->name);
        return true;
    }

}

/****************************************************************************/
/* Suspend os level task routine                                            */
/****************************************************************************/
bool os_task_suspend_RTT(void *p_handle)
{
    rt_thread_suspend((rt_thread_t)p_handle);
    rt_schedule();

    return true;
}

/****************************************************************************/
/* Resume os level task routine                                             */
/****************************************************************************/
bool os_task_resume_RTT(void *p_handle)
{
    rt_thread_resume((rt_thread_t)p_handle);
    rt_schedule();

    return true;
}

/****************************************************************************/
/* Yield current os level task routine                                      */
/****************************************************************************/
bool os_task_yield_RTT(void)
{
    rt_thread_yield();

    return true;
}

/****************************************************************************/
/* Get current os level task routine handle                                 */
/****************************************************************************/
bool os_task_handle_get_RTT(void **pp_handle)
{
    if (pp_handle == NULL)
    {
        return false;
    }

    *pp_handle = (void *)rt_thread_self();

    return true;
}


/****************************************************************************/
/* Get os level task routine priority                                       */
/****************************************************************************/
bool os_task_priority_get_RTT(void *p_handle, uint16_t *p_priority)
{
    if (p_priority == NULL)
    {
        return false;
    }

    uint16_t priority;
    rt_thread_t obj;

    if (p_handle != NULL)
    {
        obj = (rt_thread_t)p_handle;
    }
    else
    {
        obj = rt_thread_self();
    }

    priority = obj->current_priority ;

    *p_priority = priority_convert_rtt2osif(priority);

    return true;
}

/****************************************************************************/
/* Set os level task routine priority                                       */
/****************************************************************************/
bool os_task_priority_set_RTT(void *p_handle, uint16_t priority)
{
    uint16_t switch_priority = priority_convert_osif2rtt(priority);
    rt_thread_t obj;

    if (p_handle != NULL)
    {
        obj = (rt_thread_t)p_handle;
    }
    else
    {
        obj = rt_thread_self();
    }

    rt_thread_control(obj, RT_THREAD_CTRL_CHANGE_PRIORITY, &switch_priority);

    return true;
}

/****************************************************************************/
/* Take notify in target task                                              */
/****************************************************************************/
bool os_task_notify_take_RTT(long xClearCountOnExit, uint32_t xTicksToWait,
                             uint32_t *p_notify)
{
    rt_thread_t obj = rt_thread_self();
    for (uint8_t i = 0; i < TASK_SEM_ARRAY_NUMBER; i++)
    {
        if (task_sem_array[i].task_handle == obj)
        {
            rt_sem_t sem = task_sem_array[i].sem_handle;
            *p_notify = sem->value;
            rt_sem_take(sem, xTicksToWait);
            if (xClearCountOnExit == true)
            {
                rt_sem_control(sem, RT_IPC_CMD_RESET, 0);
            }
            return true;
        }
    }

    return false;
}

/****************************************************************************/
/* Give notify in target task                                              */
/****************************************************************************/
bool os_task_notify_give_RTT(void *p_handle)
{
    for (uint8_t i = 0; i < TASK_SEM_ARRAY_NUMBER; i++)
    {
        if (task_sem_array[i].task_handle == p_handle)
        {
            rt_sem_t sem = task_sem_array[i].sem_handle;
            rt_sem_release(sem);

            return true;
        }
    }

    return false;
}

/****************************************************************************/
/* Create signal to target task                                               */
/****************************************************************************/
bool os_task_signal_create_RTT(void *p_handle, uint32_t count)
{
    uint8_t i = 0;
    for (i = 0; i < TASK_SEM_ARRAY_NUMBER; i++)
    {
        if (task_sem_array[i].task_handle == NULL && task_sem_array[i].sem_handle == NULL)
        {
            break;
        }
    }

    rt_sem_t ret = rt_sem_create("sem", count, RT_IPC_FLAG_FIFO);

    if (ret != RT_NULL && i < TASK_SEM_ARRAY_NUMBER)
    {
        rt_base_t lock = rt_hw_interrupt_disable();
        task_sem_array[i].task_handle = p_handle;
        task_sem_array[i].sem_handle = ret;
        rt_hw_interrupt_enable(lock);

        return true;
    }
    else
    {
        return false;
    }
}

/****************************************************************************/
/* Send signal to target task                                               */
/****************************************************************************/
bool os_task_signal_send_RTT(void *p_handle, uint32_t signal)
{
    return true;
}

/****************************************************************************/
/* Receive signal in target task                                            */
/****************************************************************************/
bool os_task_signal_recv_RTT(uint32_t *p_signal, uint32_t wait_ms)
{
    return true;
}




/****************************************************************************/
/* Clear signal in target task                                              */
/****************************************************************************/
bool os_task_signal_clear_RTT(void *p_handle)
{
    return true;
}



/****************************************************************************/
/* Dump current all task status                                             */
/****************************************************************************/
void os_task_status_dump_RTT(void)
{
    RT_ASSERT(RT_NULL != RT_NULL);
    return;
}

/****************************************************************************/
/* Lock critical section                                                    */
/****************************************************************************/
uint32_t os_lock_RTT(void)
{
    return rt_hw_interrupt_disable();
}

/****************************************************************************/
/* Unlock critical section                                                  */
/****************************************************************************/
void os_unlock_RTT(uint32_t flags)
{
    rt_hw_interrupt_enable(flags);
}

/****************************************************************************/
/* Create counting semaphore                                                */
/****************************************************************************/
bool os_sem_create_RTT(void **pp_handle, const char *p_name, uint32_t init_count,
                       uint32_t max_count)
{
    if (pp_handle == NULL)
    {
        return false;
    }

    rt_sem_t ret;
    ret = rt_sem_create("sem", init_count, RT_IPC_FLAG_FIFO);

    if (ret != NULL)
    {
        *pp_handle = (void *)ret;
        return true;
    }
    else
    {
        return false;
    }
}

/****************************************************************************/
/* Delete counting semaphore                                                */
/****************************************************************************/
bool os_sem_delete_RTT(void *p_handle)
{
    rt_sem_delete((rt_sem_t)p_handle);

    return true;
}

/****************************************************************************/
/* Take counting semaphore                                                  */
/****************************************************************************/
bool os_sem_take_RTT(void *p_handle, uint32_t wait_ms)
{
    rt_err_t ret;

    if (rt_interrupt_get_nest() != 0)
    {
        return false;
    }

    if (os_sched_is_start() == false)
    {
        return true;
    }

    if (wait_ms == 0xFFFFFFFFUL)
    {
        ret = rt_sem_take((rt_sem_t)p_handle, RT_WAITING_FOREVER);
    }
    else
    {
        rt_tick_t wait_ticks = (rt_tick_t)((wait_ms + OSIF_TICK_PERIOD_MS - 1) / OSIF_TICK_PERIOD_MS);
        ret = rt_sem_take((rt_sem_t)p_handle, wait_ticks);
    }

    if (ret == RT_EOK)
    {
        return true;
    }
    else
    {
        return false;
    }
}

/****************************************************************************/
/* Give counting semaphore                                                  */
/****************************************************************************/
bool os_sem_give_RTT(void *p_handle)
{
    rt_err_t ret;
    ret = rt_sem_release((rt_sem_t)p_handle);

    if (ret == RT_EOK)
    {
        return true;
    }
    else
    {
        return false;
    }
}

/****************************************************************************/
/* Create recursive mutex                                                   */
/****************************************************************************/
bool os_mutex_create_RTT(void **pp_handle)
{
    if (pp_handle == NULL)
    {
        return false;
    }
    rt_mutex_t ret;

    ret = rt_mutex_create("mutex", RT_IPC_FLAG_FIFO);
    if (ret != NULL)
    {
        *pp_handle = (void *)ret;
        return true;
    }
    else
    {
        return false;
    }
}

/****************************************************************************/
/* Delete recursive mutex                                                   */
/****************************************************************************/
bool os_mutex_delete_RTT(void *p_handle)
{
    rt_err_t ret;
    ret = rt_mutex_delete((rt_mutex_t)p_handle);
    if (ret == RT_EOK)
    {
        return true;
    }
    else
    {
        /* Do not delete mutex if held by a task */
        return false;
    }
}

/****************************************************************************/
/* Take recursive mutex                                                     */
/****************************************************************************/
bool os_mutex_take_RTT(void *p_handle, uint32_t wait_ms)
{
    rt_err_t ret;

    if (rt_interrupt_get_nest() != 0)
    {
        return false;
    }

    if (os_sched_is_start() == false)
    {
        return true;
    }

    if (wait_ms == 0xFFFFFFFFUL)
    {
        ret = rt_mutex_take((rt_mutex_t)p_handle, RT_WAITING_FOREVER);
    }
    else
    {
        rt_tick_t wait_ticks = (rt_tick_t)((wait_ms + OSIF_TICK_PERIOD_MS - 1) / OSIF_TICK_PERIOD_MS);
        ret = rt_mutex_take((rt_mutex_t)p_handle, wait_ticks);
    }

    if (ret == RT_EOK)
    {
        return true;
    }
    else
    {
        return false;
    }
}

/****************************************************************************/
/* Give recursive mutex                                                     */
/****************************************************************************/
bool os_mutex_give_RTT(void *p_handle)
{
    rt_err_t ret;

    if (rt_interrupt_get_nest() != 0)
    {
        return false;
    }

    if (os_sched_is_start() == false)
    {
        return true;
    }

    ret = rt_mutex_release((rt_mutex_t)p_handle);

    if (ret == RT_EOK)
    {
        return true;
    }
    else
    {
        return false;
    }
}

/****************************************************************************/
/* Create inter-thread message queue                                        */
/****************************************************************************/
bool os_msg_queue_create_intern_RTT(void **pp_handle, const char *p_name, uint32_t msg_num,
                                    uint32_t msg_size, const char *p_func, uint32_t file_line)
{
    if (pp_handle == NULL)
    {
        return false;
    }

    rt_mq_t ret;
    ret = rt_mq_create("queue", msg_size, msg_num, RT_IPC_FLAG_FIFO);
    if (ret != NULL)
    {
        ret->msg_size = msg_size;
        *pp_handle = (void *)ret;
        return true;
    }
    else
    {
        APP_PRINT_ERROR2("msg queue create fail, func: %x, line: %d", TRACE_STRING(p_func), file_line);
        return false;
    }
}

/****************************************************************************/
/* Delete inter-thread message queue                                        */
/****************************************************************************/
bool os_msg_queue_delete_intern_RTT(void *p_handle, const char *p_func, uint32_t file_line)
{
    rt_err_t ret;
    ret = rt_mq_delete((rt_mq_t)p_handle);

    return true;
}

/****************************************************************************/
/* Peek inter-thread message queue's pending but not received msg number    */
/****************************************************************************/
bool os_msg_queue_peek_intern_RTT(void *p_handle, uint32_t *p_msg_num,
                                  const char *p_func, uint32_t file_line)
{
    rt_mq_t obj;
    rt_base_t lock;

    if (p_handle)
    {
        /* disable interrupt */
        lock = rt_hw_interrupt_disable();

        obj = (rt_mq_t)p_handle;
        *p_msg_num = obj->entry;

        /* enable interrupt */
        rt_hw_interrupt_enable(lock);

    }

    return true;
}

/****************************************************************************/
/* Send inter-thread message                                                */
/****************************************************************************/
bool os_msg_send_intern_RTT(void *p_handle, void *p_msg, uint32_t wait_ms,
                            const char *p_func, uint32_t file_line)
{
    rt_err_t ret;
    rt_base_t lock;
    rt_mq_t obj = (rt_mq_t)p_handle;;
    rt_tick_t wait_ticks;

    if (wait_ms == 0xFFFFFFFFUL)
    {
        ret = rt_mq_send_wait(obj, p_msg, obj->msg_size, RT_WAITING_FOREVER);
    }
    else
    {
        wait_ticks = (rt_tick_t)((wait_ms + OSIF_TICK_PERIOD_MS - 1) / OSIF_TICK_PERIOD_MS);
        ret = rt_mq_send_wait(obj, p_msg, obj->msg_size, wait_ticks);
    }


    if (ret == RT_EOK)
    {
        return true;
    }
    else
    {
        return false;
    }
}

/****************************************************************************/
/* Receive inter-thread message                                             */
/****************************************************************************/
bool os_msg_recv_intern_RTT(void *p_handle, void *p_msg, uint32_t wait_ms,
                            const char *p_func, uint32_t file_line)
{
    rt_err_t ret;
    rt_base_t lock;
    rt_mq_t obj = (rt_mq_t)p_handle;;
    rt_tick_t wait_ticks;

    if (wait_ms == 0xFFFFFFFFUL)
    {
        ret = rt_mq_recv(obj, p_msg, obj->msg_size, RT_WAITING_FOREVER);
    }
    else
    {
        wait_ticks = (rt_tick_t)((wait_ms + OSIF_TICK_PERIOD_MS - 1) / OSIF_TICK_PERIOD_MS);
        ret = rt_mq_recv(obj, p_msg, obj->msg_size, wait_ticks);
    }


    if (ret == RT_EOK)
    {
        return true;
    }
    else
    {
        return false;
    }
}

/****************************************************************************/
/* Peek inter-thread message                                                */
/****************************************************************************/
bool os_msg_peek_intern_RTT(void *p_handle, void *p_msg, uint32_t wait_ms,
                            const char *p_func, uint32_t file_line)
{
    //TODO
    RT_ASSERT(RT_NULL != RT_NULL);
    return true;
}

/****************************************************************************/
/* Allocate memory                                                          */
/****************************************************************************/
void *os_mem_alloc_intern_RTT(RAM_TYPE ram_type, size_t size,
                              const char *p_func, uint32_t file_line)
{
    //rt_kprintf("type = %d, size = %d \n", ram_type, size);
    if (ram_type == RAM_TYPE_DATA_ON)
    {
        return rt_memheap_alloc(data_on_heap, size);
    }
    else if (ram_type == RAM_TYPE_BUFFER_ON)
    {
        return rt_memheap_alloc(&buffer_on_heap, size);
    }
    else if (ram_type == RAM_TYPE_EXT_DATA_SRAM)
    {
        return rt_memheap_alloc(&ext_data_sram_heap, size);
    }
    else if (ram_type == RAM_TYPE_NUM)
    {
        RT_ASSERT(RT_NULL != RT_NULL);
        return 0;
    }
    else
    {
        RT_ASSERT(RT_NULL != RT_NULL);
        return 0;
    }
}

void *os_mem_zalloc_intern_RTT(RAM_TYPE ram_type, size_t size,
                               const char *p_func, uint32_t file_line)
{
    void *p;

    p = os_mem_alloc_intern_RTT(ram_type, size, __func__, __LINE__);
    if (p == NULL)
    {
        OSIF_PRINT_ERROR5("os_mem_zalloc_intern: %s<%u> ram type %u, required size %u, unused size %u",
                          TRACE_STRING(p_func), file_line, ram_type, size, os_mem_peek(ram_type));
    }
    else
    {
        memset(p, 0, size);
    }

    return p;
}


/****************************************************************************/
/* Allocate aligned memory                                                  */
/****************************************************************************/
void *os_mem_aligned_alloc_intern_RTT(RAM_TYPE ram_type, size_t size, uint8_t alignment,
                                      const char *p_func, uint32_t file_line)
{
    void *p = NULL;
    void *p_aligned = NULL;

    if (alignment == 0)
    {
        alignment = RT_ALIGN_SIZE;
    }

    if (ram_type == RAM_TYPE_DATA_ON)
    {
        p = rt_memheap_alloc(data_on_heap, size);
    }
    else if (ram_type == RAM_TYPE_BUFFER_ON)
    {
        p = rt_memheap_alloc(&buffer_on_heap, size);
    }
    else if (ram_type == RAM_TYPE_EXT_DATA_SRAM)
    {
        p = rt_memheap_alloc(&ext_data_sram_heap, size);
    }
    else if (ram_type == RAM_TYPE_NUM)
    {
        RT_ASSERT(RT_NULL != RT_NULL);
        return 0;
    }
    else
    {
        RT_ASSERT(RT_NULL != RT_NULL);
        return 0;
    }

    if (p != RT_NULL)
    {
        p_aligned = (void *)(((size_t)p + sizeof(void *) + alignment) & ~(alignment - 1));

        rt_memcpy((uint8_t *)p_aligned - sizeof(void *), &p, sizeof(void *));
    }
    else
    {
        return RT_NULL;
    }

    return p_aligned;
}

/****************************************************************************/
/* Free memory                                                              */
/****************************************************************************/
void os_mem_free_RTT(void *p_block)
{
    rt_memheap_free(p_block);
    return;
}

/****************************************************************************/
/* Free aligned memory                                                      */
/****************************************************************************/
void os_mem_aligned_free_RTT(void *p_block)
{
    if (p_block != RT_NULL)
    {
        void *p;

        rt_memcpy(&p, (uint8_t *)p_block - sizeof(void *), sizeof(void *));
        rt_memheap_free(p);
    }
    return;
}

/****************************************************************************/
/* Peek unused (available) memory size                                      */
/****************************************************************************/
size_t os_mem_peek_RTT(RAM_TYPE ram_type)
{
    if (ram_type == RAM_TYPE_DATA_ON)
    {
        return data_on_heap->available_size;
    }
    else if (ram_type == RAM_TYPE_BUFFER_ON)
    {
        return buffer_on_heap.available_size;
    }
    else if (ram_type == RAM_TYPE_EXT_DATA_SRAM)
    {
        return ext_data_sram_heap.available_size;
    }
    else if (ram_type == RAM_TYPE_NUM)
    {
        RT_ASSERT(RT_NULL != RT_NULL);
        return 0;
    }
    else
    {
        RT_ASSERT(RT_NULL != RT_NULL);
        return 0;
    }
    RT_ASSERT(RT_NULL != RT_NULL);
    return 0;
}

void os_mem_peek_printf_RTT(void)
{
    OS_PRINT_TRACE3("heap memory scan:data on %d, buffer on %d, ext data sram %d",
                    os_mem_peek_RTT(RAM_TYPE_DATA_ON), os_mem_peek_RTT(RAM_TYPE_BUFFER_ON),
                    os_mem_peek_RTT(RAM_TYPE_EXT_DATA_SRAM));
}

/****************************************************************************/
/* Check os heap usage                                                      */
/****************************************************************************/
void os_mem_check_heap_usage_RTT(void)
{
//    extern void check_heap_usage(void);
//    check_heap_usage();
    RT_ASSERT(RT_NULL != RT_NULL);
    return;
}

/****************************************************************************/
/* Get software timer ID                                                    */
/****************************************************************************/
typedef struct timer_info
{
    void *timer_handle;
    uint32_t timer_id;
} Timer_Info;

Timer_Info timer_number_array[TIMER_NUMBER_MAX];

bool os_timer_id_get_RTT(void **pp_handle, uint32_t *p_timer_id)
{
    if (pp_handle == NULL)
    {
        return false;
    }

    if (*pp_handle == NULL)
    {
        return false;
    }

    for (uint8_t i = 0; i < TIMER_NUMBER_MAX; i++)
    {
        if (timer_number_array[i].timer_handle == *pp_handle)
        {
            *p_timer_id = timer_number_array[i].timer_id;
            return true;
        }
    }

    return false;
}


/****************************************************************************/
/* Create software timer                                                    */
/****************************************************************************/
bool os_timer_create_RTT(void **pp_handle, const char *p_timer_name, uint32_t timer_id,
                         uint32_t interval_ms, bool reload, void (*p_timer_callback)(void *))
{
    if (pp_handle == NULL)
    {
        LOG_E("todo: OS_EXE_ERROR_HANDLE_IS_NULL!\n");
        return false;
    }

    if (p_timer_callback == NULL)
    {
        LOG_E("todo: OS_EXE_ERROR_TIMER_CALLBACK_IS_NULL!\n");
        return false;
    }

    rt_timer_t ret = NULL;
    uint8_t i = 0;

    if (*pp_handle == NULL)
    {
        rt_tick_t timer_ticks = (rt_tick_t)((interval_ms + OSIF_TICK_PERIOD_MS - 1) / OSIF_TICK_PERIOD_MS);

        for (i = 0; i < TIMER_NUMBER_MAX; i++)
        {
            if (timer_number_array[i].timer_handle == 0)
            {
                break;
            }
        }

#ifdef RT_USING_TIMER_SOFT
        if (reload == true)
        {
            ret = rt_timer_create(p_timer_name, p_timer_callback, NULL, timer_ticks,
                                  RT_TIMER_FLAG_PERIODIC | RT_TIMER_FLAG_SOFT_TIMER);
        }
        else
        {
            ret = rt_timer_create(p_timer_name, p_timer_callback, NULL, timer_ticks,
                                  RT_TIMER_FLAG_ONE_SHOT | RT_TIMER_FLAG_SOFT_TIMER);
        }
#else
        if (reload == true)
        {
            ret = rt_timer_create(p_timer_name, p_timer_callback, NULL, timer_ticks,
                                  RT_TIMER_FLAG_PERIODIC);
        }
        else
        {
            ret = rt_timer_create(p_timer_name, p_timer_callback, NULL, timer_ticks,
                                  RT_TIMER_FLAG_ONE_SHOT);
        }
#endif

        if (ret != RT_NULL)
        {
            /* disable interrupt */
            rt_base_t lock = rt_hw_interrupt_disable();

            ret->parameter = (void *)ret;
            if (i < TIMER_NUMBER_MAX)
            {
                timer_number_array[i].timer_handle = (void *)ret;
                timer_number_array[i].timer_id = timer_id;
            }

            /* enable interrupt */
            rt_hw_interrupt_enable(lock);

            *pp_handle = (void *)ret;

            //LOG_I("[create]timer = 0x%x, name = %s!\n", *pp_handle, p_timer_name);
            return true;
        }
        else
        {
            //DBG_DIRECT("RTT Timer %s fail", p_timer_name);
            return false;
        }
    }
    else
    {
        LOG_E("todo: OS_EXE_ERROR_PTR_HANDLE_NOT_NULL!\n");
        return false;
    }
}

/****************************************************************************/
/* Start software timer                                                     */
/****************************************************************************/
bool os_timer_start_RTT(void **pp_handle)
{
    if (pp_handle == NULL)
    {
        return false;
    }

    if (*pp_handle == NULL)
    {
        return false;
    }

    rt_err_t ret;
    rt_timer_queue_cb cb;

    cb.timer  = (rt_timer_t) * pp_handle;
    cb.cb_num = RT_TIMER_CMD_START;
    ret = rt_mq_send(rt_timer_queue, &cb, sizeof(rt_timer_queue_cb));

    if (ret == RT_EOK)
    {
        return true;
    }
    else
    {
        return false;
    }
}

/****************************************************************************/
/* Restart software timer                                                   */
/****************************************************************************/
bool os_timer_restart_RTT(void **pp_handle, uint32_t interval_ms)
{
    if (pp_handle == NULL)
    {
        return false;
    }

    if (*pp_handle == NULL)
    {
        return false;
    }

    rt_err_t ret;
    rt_timer_queue_cb cb;
    rt_tick_t timer_ticks = (rt_tick_t)((interval_ms + OSIF_TICK_PERIOD_MS - 1) / OSIF_TICK_PERIOD_MS);

    cb.timer  = (rt_timer_t) * pp_handle;
    cb.cb_num = RT_TIMER_CMD_RESTART;
    cb.change_tick = (void *)timer_ticks;
    ret = rt_mq_send(rt_timer_queue, &cb, sizeof(rt_timer_queue_cb));

    if (ret == RT_EOK)
    {
        return true;
    }
    else
    {
        return false;
    }
}

/****************************************************************************/
/* Stop software timer                                                      */
/****************************************************************************/
bool os_timer_stop_RTT(void **pp_handle)
{
    if (pp_handle == NULL)
    {
        return false;
    }

    if (*pp_handle == NULL)
    {
        return false;
    }

    rt_err_t ret;
    rt_timer_queue_cb cb;

    cb.timer  = (rt_timer_t) * pp_handle;
    cb.cb_num = RT_TIMER_CMD_STOP;
    ret = rt_mq_send(rt_timer_queue, &cb, sizeof(rt_timer_queue_cb));

    if (ret == RT_EOK)
    {
        return true;
    }
    else
    {
        return false;
    }
}

/****************************************************************************/
/* Delete software timer                                                    */
/****************************************************************************/
bool os_timer_delete_RTT(void **pp_handle)
{
    if (pp_handle == NULL)
    {
        return false;
    }

    if (*pp_handle == NULL)
    {
        return false;
    }

    /* disable interrupt */
    rt_base_t lock = rt_hw_interrupt_disable();
    rt_timer_t obj = (rt_timer_t)(*pp_handle);
    for (uint8_t i = 0; i < TIMER_NUMBER_MAX; i++)
    {
        if (timer_number_array[i].timer_handle == obj)
        {
            timer_number_array[i].timer_handle = 0;
        }
    }
    /* enable interrupt */
    rt_hw_interrupt_enable(lock);

    rt_err_t ret;
    rt_timer_queue_cb cb;

    cb.timer  = (rt_timer_t) * pp_handle;
    cb.cb_num = RT_TIMER_CMD_DEL;
    ret = rt_mq_send(rt_timer_queue, &cb, sizeof(rt_timer_queue_cb));

    if (ret == RT_EOK)
    {
        return OS_EXE_SUCCESS;
    }
    else
    {
        return OS_EXE_FAIL;
    }
}

/****************************************************************************/
/* Pend function call software timer                                       */
/****************************************************************************/
rt_mq_t rt_timer_queue;
rt_thread_t rt_timer_task;

static void cmd_proc(rt_timer_queue_cb *cb, uint8_t cmd)
{
    rt_timer_t timer;
    timer = cb->timer;
    rt_base_t lock;

    switch (cmd)
    {
    case RT_TIMER_CMD_START:
        rt_timer_start(timer);
        break;
    case RT_TIMER_CMD_STOP:
        rt_timer_stop(timer);
        break;
    case RT_TIMER_CMD_RESTART:
        rt_timer_stop(timer);
        rt_timer_control(timer, RT_TIMER_CTRL_SET_TIME, &(cb->change_tick));
        rt_timer_start(timer);
        break;
    case RT_TIMER_CMD_DEL:
        rt_timer_delete(timer);
        break;
    case RT_TIMER_EXCUTE_CALLBACK:
        cb->pxCallbackFunction(cb->pend_para1, cb->pend_para2);
        break;
    default:
        break;
    }
}

static void timer_cmd_proc(rt_timer_queue_cb *cb)
{
    cmd_proc(cb, cb->cb_num);
}

void timer_task(void *pa)
{
    rt_timer_queue_cb cb_msg;
    while (1)
    {
        if (rt_mq_recv(rt_timer_queue, &cb_msg, sizeof(cb_msg), RT_WAITING_FOREVER) == RT_EOK)
        {
            timer_cmd_proc(&cb_msg);
        }
    }
}

rt_err_t xPendFunctionCall(PendedFunctionOS_t xFunctionToPend, void *para1,
                           uint32_t para2)
{
    rt_timer_queue_cb cb;
    rt_err_t ret;

    cb.pxCallbackFunction = xFunctionToPend;
    cb.pend_para1 = para1;
    cb.pend_para2 = para2;
    cb.cb_num = RT_TIMER_EXCUTE_CALLBACK;

    ret = rt_mq_send(rt_timer_queue, &cb, sizeof(rt_timer_queue_cb));

    return ret;
}

bool os_timer_pend_function_call_RTT(void (*p_pend_function)(void *, uint32_t),
                                     void *pvParameter1, uint32_t ulParameter2)
{
    rt_err_t ret = xPendFunctionCall(p_pend_function, pvParameter1, ulParameter2);
    if (ret == RT_EOK)
    {
        return true;
    }
    else
    {
        return false;
    }
}

/****************************************************************************/
/* Get software timer is active                                                */
/****************************************************************************/
bool os_timer_is_timer_active_RTT(void **pp_handle)
{
    rt_base_t lock;
    rt_timer_t obj;

    if (pp_handle && *pp_handle)
    {
        /* disable interrupt */
        lock = rt_hw_interrupt_disable();

        obj = (rt_timer_t)(*pp_handle);
        RT_ASSERT(rt_object_get_type(&obj->parent) == RT_Object_Class_Timer);

        if (obj->parent.flag & RT_TIMER_FLAG_ACTIVATED)
        {
            /*timer is start and run*/
            rt_hw_interrupt_enable(lock);
            return true;
        }
        else
        {
            /*timer is stop*/
            rt_hw_interrupt_enable(lock);
            return false;
        }
    }
    else
    {
        return false;
    }
}

/****************************************************************************/
/* Get software timer state                                                */
/****************************************************************************/
bool os_timer_state_get_RTT(void **pp_handle, uint32_t *p_timer_state)
{
    rt_base_t lock;
    rt_timer_t obj;

    if (pp_handle && *pp_handle)
    {
        /* disable interrupt */
        lock = rt_hw_interrupt_disable();

        obj = (rt_timer_t)(*pp_handle);
        RT_ASSERT(rt_object_get_type(&obj->parent) == RT_Object_Class_Timer);

        if (obj->parent.flag & RT_TIMER_FLAG_ACTIVATED)
        {
            /*timer is start and run*/
            *(rt_tick_t *)p_timer_state = RT_TIMER_FLAG_ACTIVATED;
        }
        else
        {
            /*timer is stop*/
            *(rt_tick_t *)p_timer_state = RT_TIMER_FLAG_DEACTIVATED;
        }

        /* enable interrupt */
        rt_hw_interrupt_enable(lock);

    }

    return true;
}

/****************************************************************************/
/* Get software timer is period timer or not                                */
/****************************************************************************/
bool os_timer_get_auto_reload_RTT(void **pp_handle, long *p_autoreload)
{
    if (pp_handle == NULL)
    {
        return false;
    }

    if (*pp_handle == NULL)
    {
        return false;
    }

    if (p_autoreload == NULL)
    {
        return false;
    }

    rt_timer_t timer = *pp_handle;
    register rt_base_t level;

    /* timer check */
    RT_ASSERT(timer != RT_NULL);
    RT_ASSERT(rt_object_get_type(&timer->parent) == RT_Object_Class_Timer);

    level = rt_hw_interrupt_disable();
    if (timer->parent.flag & RT_TIMER_FLAG_PERIODIC)
    {
        *p_autoreload = true;
    }
    else
    {
        *p_autoreload = false;
    }
    rt_hw_interrupt_enable(level);

    return true;
}

/****************************************************************************/
/* Dump software timer                                                      */
/****************************************************************************/
bool os_timer_dump_RTT(void)
{
    RT_ASSERT(RT_NULL != RT_NULL);
    return true;
}

/****************************************************************************/
/* Get software timer number                                                */
/****************************************************************************/
bool os_timer_get_timer_number_RTT(void **pp_handle, uint8_t *p_timer_number)
{
    if (pp_handle && *pp_handle)
    {
        for (uint8_t i = 0; i < TIMER_NUMBER_MAX; i++)
        {
            if (timer_number_array[i].timer_handle == *pp_handle)
            {
                *p_timer_number = i;
                return true;
            }
        }
    }
    else
    {
        *p_timer_number = 0xFF;
    }

    return false;
}

/****************************************************************************/
/* Init software timer pool                                                */
/****************************************************************************/
void os_timer_init_RTT(void)
{
    rt_timer_queue = rt_mq_create("timer_queue", sizeof(rt_timer_queue_cb), TIMER_CMD_QUEUE_LENGTH,
                                  RT_IPC_FLAG_FIFO);
    rt_timer_task = rt_thread_create("timer_task", timer_task, NULL, 1024, 0, 2);

    if (rt_timer_task != RT_NULL)
    {
        rt_thread_startup(rt_timer_task);
    }

    return;
}



/****************************************************************************/
/* Create isr trace                                                         */
/****************************************************************************/
bool os_trace_isr_create_RTT(void **pp_handle, const char *p_name, uint32_t priority)
{
    return true;
}

/****************************************************************************/
/* Create isr trace begin                                                   */
/****************************************************************************/
bool os_trace_isr_begin_RTT(void *pp_handle)
{
    return true;
}

/****************************************************************************/
/* Create isr trace end                                                     */
/****************************************************************************/
bool os_trace_isr_end_RTT(void)
{
    return true;
}

/****************************************************************************/
/*dlps restore os kernel scheduler processing                               */
/****************************************************************************/
void os_pm_bottom_half_RTT(void (*pend_func)(void))
{
    if (!os_timer_pend_function_call((void (*)(void *, uint32_t))pend_func, NULL, 0))
    {
        OSIF_PRINT_ERROR0("osif_pm_bottom_half function pend fail");
    }
}

void os_pm_return_to_idle_task_RTT(void)
{
    extern uint32_t TopOfIdleTaskStack;
    extern uint32_t IdleTaskStackLim;

    __set_MSP(*(volatile uint32_t *)SCB->VTOR);

    __ISB();

    __set_PSPLIM(IdleTaskStackLim);

    __ISB();

    __set_CONTROL(__get_CONTROL() | BIT1);

    __ISB();

    __set_PSP(TopOfIdleTaskStack);

    __ISB();

    extern void rt_thread_idle_entry(void *parameter);
    rt_thread_idle_entry((void *)TopOfIdleTaskStack);
}

extern PMCheckResult os_pm_check(uint32_t *wakeup_time_diff);
extern void os_pm_store(void);
extern void os_pm_restore(void);

void os_pm_init_RTT(void)
{
    power_manager_slave_register_function_to_return(os_pm_return_to_idle_task_RTT);
    platform_pm_register_schedule_bottom_half_callback_func(os_pm_bottom_half_RTT);

    platform_pm_register_callback_func_with_priority((void *)os_pm_check, PLATFORM_PM_CHECK, 1);
    platform_pm_register_callback_func_with_priority((void *)os_pm_store, PLATFORM_PM_STORE, 1);
    platform_pm_register_callback_func_with_priority((void *)os_pm_restore, PLATFORM_PM_RESTORE, 1);
}

/****************************************************************************/
/*Restore os kernel scheduler                                               */
/****************************************************************************/
bool os_sched_restore_RTT(void)
{
    vPortStartSchedulerInRestore();

#if ( configENABLE_TRUSTZONE == 1 )
    rt_trustzone_init();
    extern rt_ubase_t rt_trustzone_current_context;
    rt_trustzone_context_load(rt_trustzone_current_context);
#endif

#if ( configENABLE_FPU == 1 )
    extern void prvSetupFPU(void);
    prvSetupFPU();
#endif

    return true;
}


/****************************************************************************/
/*os kernel systick handler                                                 */
/****************************************************************************/
void os_systick_handler_RTT(void)
{
    rt_tick_t old_tick = rt_tick_get();
    rt_tick_set(old_tick + 1);
}

uint32_t os_sys_tick_rate_get_RTT(void)
{
    return (uint32_t)OSIF_TICK_RATE_HZ;
}

uint32_t os_sys_tick_clk_get_RTT(void)
{
    return (uint32_t)OSIF_SYSTICK_CLOCK_HZ;
}


uint64_t os_sys_tick_increase_RTT(uint32_t tick_increment)
{
    rt_tick_t old_tick = rt_tick_get();
    rt_tick_set(old_tick + tick_increment);
    return old_tick;
}

/****************************************************************************/
/* Get software timer pool next expire value                                 */
/****************************************************************************/
#ifdef RT_USING_TIMER_SOFT
extern rt_list_t rt_timer_list[RT_TIMER_SKIP_LIST_LEVEL];
extern rt_list_t rt_soft_timer_list[RT_TIMER_SKIP_LIST_LEVEL];

static rt_tick_t rt_timer_list_next_timeout_tick(rt_list_t timer_list[],
                                                 PlatformExcludedHandleType type)
{
    struct rt_timer *timer;
    register rt_base_t level;
    rt_list_t *n;

    /* disable interrupt */
    level = rt_hw_interrupt_disable();

    if (!rt_list_isempty(&timer_list[RT_TIMER_SKIP_LIST_LEVEL - 1]))
    {
        for (n = timer_list[RT_TIMER_SKIP_LIST_LEVEL - 1].next;
             n != &(timer_list[RT_TIMER_SKIP_LIST_LEVEL - 1]);)
        {
            bool handle_checked = true;
            timer = rt_list_entry(n, struct rt_timer, row[RT_TIMER_SKIP_LIST_LEVEL - 1]);
            // search if itemOwner is in the exclude_handle
            T_OS_QUEUE_ELEM *p_cur_queue_item = lpm_excluded_handle[type].p_first;
            while (p_cur_queue_item != NULL)
            {
                void *cur_excluded_handle = *(((PlatformPMExcludedHandleQueueElem *)p_cur_queue_item)->handle);
                if (cur_excluded_handle != NULL)
                {
                    if (type == PLATFORM_PM_EXCLUDED_TIMER)
                    {
                        bool is_auto_reload;
                        os_timer_get_auto_reload(&cur_excluded_handle, (void *)&is_auto_reload);
                        if (is_auto_reload)
                        {
                            DBG_DIRECT("[PM]!!handle=0x%x", cur_excluded_handle);
                            configASSERT(0);
                        }
                    }

                    if (timer == cur_excluded_handle)
                    {
                        handle_checked = false;
                        break;
                    }
                }
                p_cur_queue_item = p_cur_queue_item->p_next;
            }

            // calculate timediff
            if (handle_checked)
            {
                uint64_t timediff;
                rt_tick_t expire_time = timer->timeout_tick;
                rt_tick_t current_tick = rt_tick_get();

                timediff = clk_cnt_diff(current_tick, expire_time, BIT64(32));
                if (timediff >= RT_TICK_MAX / 2)
                {
                    timediff = RT_TICK_MAX;
                }

                if (type == PLATFORM_PM_EXCLUDED_TIMER)
                {
                    DBG_DIRECT("[PM] !**timer check: name:%s, now:0x%x, next:0x%x",
                               timer->parent.name,
                               current_tick,
                               expire_time);
                }
                else
                {
                    DBG_DIRECT("[PM] !**task check: name:%s, now:0x%x, next:0x%x",
                               timer->parent.name,
                               current_tick,
                               expire_time);
                }

                /* enable interrupt */
                rt_hw_interrupt_enable(level);
                return timediff;
            }

            n = n->next;
        }
    }

    /* enable interrupt */
    rt_hw_interrupt_enable(level);
    return RT_TICK_MAX;
}
#else
static rt_tick_t rt_timer_list_next_timeout_tick(rt_list_t timer_list[],
                                                 PlatformExcludedHandleType type)
{
    struct rt_timer *timer;
    register rt_base_t level;
    rt_list_t *n;

    /* disable interrupt */
    level = rt_hw_interrupt_disable();

    if (!rt_list_isempty(&timer_list[RT_TIMER_SKIP_LIST_LEVEL - 1]))
    {
        for (n = timer_list[RT_TIMER_SKIP_LIST_LEVEL - 1].next;
             n != &(timer_list[RT_TIMER_SKIP_LIST_LEVEL - 1]);)
        {
            bool handle_checked = true;
            timer = rt_list_entry(n, struct rt_timer, row[RT_TIMER_SKIP_LIST_LEVEL - 1]);
            // search if itemOwner is in the exclude_handle
            T_OS_QUEUE_ELEM *p_cur_queue_item = lpm_excluded_handle[PLATFORM_PM_EXCLUDED_TIMER].p_first;
            while (p_cur_queue_item != NULL)
            {
                void *cur_excluded_handle = *(((PlatformPMExcludedHandleQueueElem *)p_cur_queue_item)->handle);
                if (cur_excluded_handle != NULL)
                {
                    bool is_auto_reload;
                    os_timer_get_auto_reload(&cur_excluded_handle, (void *)&is_auto_reload);
                    if (is_auto_reload)
                    {
                        DBG_DIRECT("[PM]!!handle=0x%x", cur_excluded_handle);
                        configASSERT(0);
                    }

                    if (timer == cur_excluded_handle)
                    {
                        handle_checked = false;
                        break;
                    }
                }
                p_cur_queue_item = p_cur_queue_item->p_next;
            }

            p_cur_queue_item = lpm_excluded_handle[PLATFORM_PM_EXCLUDED_TASK].p_first;
            while (p_cur_queue_item != NULL)
            {
                void *cur_excluded_handle = *(((PlatformPMExcludedHandleQueueElem *)p_cur_queue_item)->handle);
                if (cur_excluded_handle != NULL)
                {
                    if (timer == cur_excluded_handle)
                    {
                        handle_checked = false;
                        break;
                    }
                }
                p_cur_queue_item = p_cur_queue_item->p_next;
            }

            // calculate timediff
            if (handle_checked)
            {
                uint64_t timediff;
                rt_tick_t expire_time = timer->timeout_tick;
                rt_tick_t current_tick = rt_tick_get();

                timediff = clk_cnt_diff(current_tick, expire_time, BIT64(32));
                if (timediff >= RT_TICK_MAX / 2)
                {
                    timediff = RT_TICK_MAX;
                }

                DBG_DIRECT("[PM] !**task check: name:%s, now:0x%x, next:0x%x",
                           timer->parent.name,
                           current_tick,
                           expire_time);

                /* enable interrupt */
                rt_hw_interrupt_enable(level);
                return timediff;
            }

            n = n->next;
        }
    }

    /* enable interrupt */
    rt_hw_interrupt_enable(level);
    return RT_TICK_MAX;
}
#endif

uint32_t os_pm_next_timeout_value_get_RTT(void)
{
    rt_tick_t next_timeout;

#ifdef RT_USING_TIMER_SOFT
    rt_tick_t next_timeout_timer = rt_timer_list_next_timeout_tick(rt_soft_timer_list,
                                                                   PLATFORM_PM_EXCLUDED_TIMER);
//    rt_timer_dump_RTT(rt_soft_timer_list);
#else
    rt_tick_t next_timeout_timer = RT_WAITING_FOREVER;
#endif

    rt_tick_t next_timeout_task = rt_timer_list_next_timeout_tick(rt_timer_list,
                                                                  PLATFORM_PM_EXCLUDED_TASK);
//    rt_timer_dump_RTT(rt_timer_list);

    if (next_timeout_task < next_timeout_timer)
    {
        next_timeout = next_timeout_task;
    }
    else
    {
        next_timeout = next_timeout_timer;
    }

    return next_timeout;
}

/****************************************************************************/
/*dlps restore os kernel scheduler processing                               */
/****************************************************************************/
bool os_register_pm_excluded_handle_RTT(void **pp_handle, PlatformExcludedHandleType type)
{
    if (pp_handle == NULL)
    {
        return false;
    }

    PlatformPMExcludedHandleQueueElem *p_handle_queue_item = (PlatformPMExcludedHandleQueueElem *)
                                                             os_mem_alloc(RAM_TYPE_DATA_ON, sizeof(PlatformPMExcludedHandleQueueElem));

    if (p_handle_queue_item == NULL)
    {
        return false;
    }

    p_handle_queue_item->handle = pp_handle;
    os_queue_in(&lpm_excluded_handle[type], p_handle_queue_item);

    return true;
}

/****************************************************************************/
/*dlps restore os kernel scheduler processing                               */
/****************************************************************************/
bool os_unregister_pm_excluded_handle_RTT(void **pp_handle, PlatformExcludedHandleType type)
{
    if (pp_handle == NULL)
    {
        return false;
    }

    T_OS_QUEUE_ELEM *p_cur_queue_elem = lpm_excluded_handle[type].p_first;
    bool check_found = false;

    while (p_cur_queue_elem != NULL)
    {
        void **p_cur_handle = ((PlatformPMExcludedHandleQueueElem *)p_cur_queue_elem)->handle;
        if (pp_handle == p_cur_handle)
        {
            check_found = true;
            os_queue_delete(&lpm_excluded_handle[type], p_cur_queue_elem);
            break;
        }
        p_cur_queue_elem = p_cur_queue_elem->p_next;
    }

    if (check_found)
    {
        os_mem_free(p_cur_queue_elem);
        return true;
    }
    else
    {
        return false;
    }
}

void osif_mem_func_init_RTT()
{
    os_mem_alloc_intern = os_mem_alloc_intern_RTT;
    os_mem_zalloc_intern = os_mem_zalloc_intern_RTT;
    os_mem_aligned_alloc_intern = os_mem_aligned_alloc_intern_RTT;
    os_mem_free = os_mem_free_RTT;
    os_mem_aligned_free = os_mem_aligned_free_RTT;
    os_mem_peek = os_mem_peek_RTT;
    os_mem_peek_printf = os_mem_peek_printf_RTT;
    os_mem_check_heap_usage = os_mem_check_heap_usage_RTT;
}

void osif_msg_func_init_RTT()
{
    os_msg_queue_create_intern = os_msg_queue_create_intern_RTT;
    os_msg_queue_delete_intern = os_msg_queue_delete_intern_RTT;
    os_msg_queue_peek_intern = os_msg_queue_peek_intern_RTT;
    os_msg_send_intern = os_msg_send_intern_RTT;
    os_msg_recv_intern = os_msg_recv_intern_RTT;
    os_msg_peek_intern = os_msg_peek_intern_RTT;
}

void osif_sched_func_init_RTT(void)
{
    os_init = os_init_RTT;
    os_vector_table_update = os_vector_table_update_RTT;

    os_delay = os_delay_RTT;
    os_sys_time_get = os_sys_time_get_RTT;
    os_sys_tick_get = os_sys_tick_get_RTT;
    os_sched_start = os_sched_start_RTT;
    os_sched_stop = os_sched_stop_RTT;
    os_sched_suspend = os_sched_suspend_RTT;
    os_sched_resume = os_sched_resume_RTT;
    os_sched_state_get = os_sched_state_get_RTT;
    os_sched_is_start = os_sched_is_start_RTT;
}

void osif_sync_func_init_RTT(void)
{
    os_lock = os_lock_RTT;
    os_unlock = os_unlock_RTT;
    os_sem_create = os_sem_create_RTT;
    os_sem_delete = os_sem_delete_RTT;
    os_sem_take = os_sem_take_RTT;
    os_sem_give = os_sem_give_RTT;
    os_mutex_create = os_mutex_create_RTT;
    os_mutex_delete = os_mutex_delete_RTT;
    os_mutex_take = os_mutex_take_RTT;
    os_mutex_give = os_mutex_give_RTT;
}

void osif_task_func_init_RTT(void)
{
    os_alloc_secure_ctx = os_alloc_secure_ctx_RTT;
    os_task_create = os_task_create_RTT;
    os_task_delete = os_task_delete_RTT;
    os_task_suspend = os_task_suspend_RTT;
    os_task_resume = os_task_resume_RTT;
    os_task_yield = os_task_yield_RTT;
    os_task_handle_get = os_task_handle_get_RTT;
    os_task_priority_get = os_task_priority_get_RTT;
    os_task_priority_set = os_task_priority_set_RTT;
    os_task_signal_create = os_task_signal_create_RTT;
    os_task_signal_send = os_task_signal_send_RTT;
    os_task_signal_recv = os_task_signal_recv_RTT;
    os_task_signal_clear = os_task_signal_clear_RTT;
    os_task_notify_take = os_task_notify_take_RTT;
    os_task_notify_give = os_task_notify_give_RTT;
    os_task_status_dump = os_task_status_dump_RTT;
}

void osif_timer_func_init_RTT(void)
{
    os_timer_id_get = os_timer_id_get_RTT;
    os_timer_create = os_timer_create_RTT;
    os_timer_start = os_timer_start_RTT;
    os_timer_restart = os_timer_restart_RTT;
    os_timer_stop = os_timer_stop_RTT;
    os_timer_delete = os_timer_delete_RTT;
    os_timer_pend_function_call = os_timer_pend_function_call_RTT;
    os_timer_is_timer_active = os_timer_is_timer_active_RTT;
    os_timer_state_get = os_timer_state_get_RTT;
    os_timer_get_auto_reload = os_timer_get_auto_reload_RTT;
    os_timer_get_timer_number = os_timer_get_timer_number_RTT;
    os_timer_dump = os_timer_dump_RTT;
    os_timer_init = os_timer_init_RTT;
}

void osif_trace_func_init_RTT(void)
{
    os_trace_isr_create = os_trace_isr_create_RTT;
    os_trace_isr_begin = os_trace_isr_begin_RTT;
    os_trace_isr_end = os_trace_isr_end_RTT;
}

void osif_pm_func_init_RTT(void)
{
    os_pm_init = os_pm_init_RTT;
    os_sched_restore = os_sched_restore_RTT;
    os_systick_handler = os_systick_handler_RTT;
    os_sys_tick_rate_get = os_sys_tick_rate_get_RTT;
    os_sys_tick_clk_get = os_sys_tick_clk_get_RTT;
    os_sys_tick_increase = os_sys_tick_increase_RTT;

    os_pm_next_timeout_value_get = os_pm_next_timeout_value_get_RTT;

    os_register_pm_excluded_handle = os_register_pm_excluded_handle_RTT;
    os_unregister_pm_excluded_handle = os_unregister_pm_excluded_handle_RTT;
}

void os_rtt_patch_init(void) APP_FLASH_TEXT_SECTION;
void os_rtt_patch_init(void)
{
    osif_mem_func_init_RTT();
    osif_msg_func_init_RTT();
    os_pool_func_init();
    os_queue_func_init();
    osif_sched_func_init_RTT();
    osif_sync_func_init_RTT();
    osif_task_func_init_RTT();
    osif_timer_func_init_RTT();
    os_trace_func_init();
    osif_pm_func_init_RTT();
}

