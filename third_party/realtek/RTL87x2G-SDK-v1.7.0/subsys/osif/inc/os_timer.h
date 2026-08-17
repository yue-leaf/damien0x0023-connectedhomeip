/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _OS_TIMER_H_
#define _OS_TIMER_H_

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \defgroup    OS_TIMER Timer Management
 *
 * \brief   Create and control software timer and timer callback functions.
 * \details The Timer Management function group allows creating, deleting, and controlling software
 *          timers in the system. The software timers can be configured as one-shot or periodic
 *          timers. When a timer expires, a callback function associated with the timer is
 *          executed.\n
 * Timers can be in the following two states:\n
 * \arg <b>Dormant</b> The timer first enters Dormant or Inactive state when created.
 *                     If a one-shot timer expires but is not restarted yet, or is stopped, the timer
 *                     will be transformed from Active state into Dormant state.
 * \arg <b>Active</b> The timer enters Active state if the timer starts or restarts. When
 *                    expired, the callback function associated with the timer will be executed.
 *
 * \image html OS-timer-state-transition.jpg "Software Timer State Transition" width=50px height=26px
 *
 */

/*============================================================================*
 *                              Functions
*============================================================================*/
/** @defgroup OS_TIMER_Exported_Functions OS Timer Exported Functions
  * \ingroup  OS_TIMER
  * @{
  */

/**
 *
 * \brief    Get the ID assigned to the timer when created.
 *
 * \param[in]   pp_handle   Pointer to the created timer handle.
 *
 * \param[out]  p_timer_id  Used to pass back the ID assigned to the timer.
 *
 * \return           The status of the timer ID getting.
 * \retval true      Timer ID was got successfully.
 * \retval false     Timer ID failed to get. It happens when pp_handle is NULL or *pp_handle is NULL.
 *
 * <b>Example usage</b>
 * \code{.c}
 * // Timer callback function.
 * void timer_callback(void *p_handle)
 * {
 *     uint32_t timer_id;
 *
 *     // Which timer expired?
 *     os_timer_id_get(&p_handle, &timer_id);
 *
 *     if (timer_id == TIMER_ID)
 *     {
 *          // Delete the specified timer.
 *          os_timer_delete(&p_handle);
 *     }
 * }
 *
 * #define TIMER_ID 1
 *
 * // Timer to be created.
 * int test(void)
 * {
 *     void *p_handle = NULL;
 *
 *     if (os_timer_create(&p_handle, "timer", TIMER_ID,
 *                        INTERVAL_MS, false, timer_callback) == true)
 *     {
 *         // Timer created successfully, start the timer.
 *         os_timer_start(&p_handle);
 *     }
 *     else
 *     {
 *         // Timer failed to create.
 *         return -1;
 *     }
 *
 *     return 0;
 * }
 * \endcode
 *
 */
extern bool (*os_timer_id_get)(void **pp_handle, uint32_t *p_timer_id);

/**
 *
 * \brief   Create a new software timer instance. This allocates the storage required by
 *          the new timer, initializes the new timer's internal state, and returns a handle
 *          by which the new timer can be referenced.
 *
 * \param[out]  pp_handle        Used to pass back a handle by which the created timer
 *                               can be referenced.
 *
 * \param[in]   p_timer_name     A descriptive name for the timer.
 *
 * \param[in]   timer_id         An identifier that is assigned to the timer being created.
 *                               Typically this would be used in the timer callback function to
 *                               identify which timer expired when the same callback function is
 *                               assigned to more than one timer.
 *
 * \param[in]   interval_ms      The timer period in milliseconds.
 *
 * \param[in]   reload           Used to set the timer as a periodic or one-shot timer.
 * \arg \c      true             Create a periodic timer.
 * \arg \c      false            Create a one-shot timer.
 *
 * \param   p_timer_callback The function to call when the timer expires. Callback functions
 *                               must have the prototype defined as 'void callback(void *)'.
 *
 * \return           The status of the timer creation.
 * \retval true      Timer was created successfully.
 * \retval false     Timer failed to create. It happens when the parameters are invalid or there is no free timer in pool.
 *
 * <b>Example usage</b>
 * \code{.c}
 * // Timer callback function.
 * void timer_callback(void *p_handle)
 * {
 *     uint32_t timer_id;
 *
 *     // Which timer expired?
 *     os_timer_id_get(&p_handle, &timer_id);
 *
 *     if (timer_id == TIMER_ID)
 *     {
 *          // Delete the specified timer.
 *          os_timer_delete(&p_handle);
 *     }
 * }
 *
 * #define TIMER_ID 1
 *
 * // Timer to be created.
 * int test(void)
 * {
 *     void *p_handle = NULL;
 *
 *     if (os_timer_create(&p_handle, "timer", TIMER_ID,
 *                        INTERVAL_MS, false, timer_callback) == true)
 *     {
 *         // Timer created successfully, start the timer.
 *         os_timer_start(&p_handle);
 *     }
 *     else
 *     {
 *         // Timer failed to create.
 *         return -1;
 *     }
 *
 *     return 0;
 * }
 * \endcode
 *
 */
extern bool (*os_timer_create)(void **pp_handle, const char *p_timer_name, uint32_t timer_id,
                               uint32_t interval_ms, bool reload, void (*p_timer_callback)(void *));

/**
 *
 * \brief    Start a timer that was previously created using the os_timer_create() API
 *           function. If the timer had already been started and was in the active state,
 *           then os_timer_start() will restart it. If the timer was in the dormant state,
 *           os_timer_start() will change it to active state and restart.
 *
 * \param[in] pp_handle Pointer to the created timer handle.
 *
 * \return           The status of the timer starting.
 * \retval true      Timer was started successfully.
 * \retval false     Timer failed to start. It happens when the parameters are invalid or timer command queue is full.
 *
 * <b>Example usage</b>
 * \code{.c}
 * // Timer callback function.
 * void timer_callback(void *p_handle)
 * {
 *     uint32_t timer_id;
 *
 *     // Which timer expired?
 *     os_timer_id_get(&p_handle, &timer_id);
 *
 *     if (timer_id == TIMER_ID)
 *     {
 *          // Delete the specified timer.
 *          os_timer_delete(&p_handle);
 *     }
 * }
 *
 * #define TIMER_ID 1
 *
 * // Timer to be created.
 * int test(void)
 * {
 *     void *p_handle = NULL;
 *
 *     if (os_timer_create(&p_handle, "timer", TIMER_ID,
 *                        INTERVAL_MS, false, timer_callback) == true)
 *     {
 *         // Timer created successfully, start the timer.
 *         os_timer_start(&p_handle);
 *     }
 *     else
 *     {
 *         // Timer failed to create.
 *         return -1;
 *     }
 *
 *     return 0;
 * }
 * \endcode
 *
 */
extern bool (*os_timer_start)(void **pp_handle);

/**
 *
 * \brief     Change timer periods and restart a timer that was previously created.
 *            If the timer had already been started and was already in the active
 *            state, then os_timer_restart() will cause the timer to re-evaluate its expiry
 *            time and restart. If the timer was in the dormant state then os_timer_restart()
 *            will change it to active state and start in the new timer period.
 *
 * \param[in] pp_handle     Pointer to the created timer handle.
 *
 * \param[in] interval_ms   The timer period in milliseconds.
 *
 * \return           The status of the timer restarting.
 * \retval true      Timer was restarted successfully.
 * \retval false     Timer failed to restart. It happens when the parameters are invalid or timer command queue is full.
 *
 * <b>Example usage</b>
 * \code{.c}
 * // Timer callback function.
 * void timer_callback(void *p_handle)
 * {
 *     uint32_t timer_id;
 *
 *     // Which timer expired?
 *     os_timer_id_get(&p_handle, &timer_id);
 *
 *     if (timer_id == TIMER_ID)
 *     {
 *          // Restart the specified timer.
 *          os_timer_restart(&p_handle);
 *     }
 * }
 *
 * #define TIMER_ID 1
 *
 * // Timer to be created.
 * int test(void)
 * {
 *     void *p_handle = NULL;
 *
 *     if (os_timer_create(&p_handle, "timer", TIMER_ID,
 *                        INTERVAL_MS, false, timer_callback) == true)
 *     {
 *         // Timer created successfully, start the timer.
 *         os_timer_start(&p_handle);
 *     }
 *     else
 *     {
 *         // Timer failed to create.
 *         return -1;
 *     }
 *
 *     return 0;
 * }
 * \endcode
 *
 */
extern bool (*os_timer_restart)(void **pp_handle, uint32_t interval_ms);

/**
 *
 * \brief     Stop a timer that was previously started using either of the os_timer_start(),
 *            os_timer_restart() API functions. Stopping a timer ensures the timer is not in
 *            the active state.
 *
 * \param[in] pp_handle Pointer to the handle of timer being stopped.
 *
 * \return           The status of the timer stopping.
 * \retval true      Timer was stopped successfully.
 * \retval false     Timer failed to stop. It happens when the parameters are invalid or timer command queue is full.
 *
 * <b>Example usage</b>
 * \code{.c}
 * // Timer callback function.
 * void timer_callback(void *p_handle)
 * {
 *     uint32_t timer_id;
 *
 *     // Which timer expired?
 *     os_timer_id_get(&p_handle, &timer_id);
 *
 *     if (timer_id == TIMER_ID)
 *     {
 *          // Delete the specified timer.
 *          os_timer_delete(&p_handle);
 *     }
 * }
 *
 * #define TIMER_ID 1
 *
 * // Timer to be created and stopped.
 * int test(void)
 * {
 *     void *p_handle = NULL;
 *
 *     if (os_timer_create(&p_handle, "timer", TIMER_ID,
 *                        INTERVAL_MS, false, timer_callback) == true)
 *     {
 *         // Timer created successfully, start the timer.
 *         os_timer_start(&p_handle);
 *     }
 *     else
 *     {
 *         // Timer failed to create.
 *         return -1;
 *     }
 *
 *     // Stop the timer before expiration.
 *     os_timer_stop(&p_handle);
 *
 *     return 0;
 * }
 * \endcode
 *
 */
extern bool (*os_timer_stop)(void **pp_handle);

/**
 *
 * \brief      Delete a timer that was previously created using the os_timer_create() API function.
 *
 * \param[in] pp_handle Pointer to the handle of timer being deleted.
 *
 * \return           The status of the timer deletion.
 * \retval true      Timer was deleted successfully.
 * \retval false     Timer failed to delete. It happens when the parameters are invalid or timer command queue is full.
 *
 * <b>Example usage</b>
 * \code{.c}
 * // Timer callback function.
 * void timer_callback(void *p_handle)
 * {
 *     uint32_t timer_id;
 *
 *     // Which timer expired?
 *     os_timer_id_get(&p_handle, &timer_id);
 *
 *     if (timer_id == TIMER_ID)
 *     {
 *          // Delete the specified timer.
 *          os_timer_delete(&p_handle);
 *     }
 * }
 *
 * #define TIMER_ID 1
 *
 * // Timer to be created and deleted.
 * int test(void)
 * {
 *     void *p_handle = NULL;
 *
 *     if (os_timer_create(&p_handle, "timer", TIMER_ID,
 *                        INTERVAL_MS, false, timer_callback) == true)
 *     {
 *         // Timer created successfully, start the timer.
 *         os_timer_start(&p_handle);
 *     }
 *     else
 *     {
 *         // Timer failed to create.
 *         return -1;
 *     }
 *
 *     return 0;
 * }
 * \endcode
 *
 */
extern bool (*os_timer_delete)(void **pp_handle);

/**
 *
 * \brief  Register pend callback function and send it to timer task to execute later.
 *
 * \param[in]  p_pend_function Pend callback function, which will be executed in SW timer task.
 * \param[in]  pvParameter1 Parameter1 for pend callback function
 * \param[in]  ulParameter2 Parameter2 for pend callback function
 *
 * \return           The status of sending timer pend callback function.
 * \retval true      Pend callback function was sent successfully.
 * \retval false     Pend callback function failed to send. It happens when timer command queue is full.
 *

 *
 */
extern bool (*os_timer_pend_function_call)(void (*p_pend_function)(void *, uint32_t),
                                           void *pvParameter1, uint32_t ulParameter2);

/**
  * @brief  Check if SW timer is active.
  * @param[in]  pp_handle Pointer to the handle of timer
  * @return Result of operation
  * \retval true  SW timer is active.
  * \retval false SW timer is not active.
  */
extern bool (*os_timer_is_timer_active)(void **pp_handle);

/**
  * @brief  Get auto reload status of SW timer.
  * @param[in]  pp_handle Pointer to the handle of timer
  * @param[out]  p_autoreload Auto reload status of SW timer
  * @return Result of operation
  * \retval true  Get auto reload status of SW timer successfully.
  * \retval false Get auto reload status of SW timer failed. It happens when the parameters are invalid.
  */
extern bool (*os_timer_get_auto_reload)(void **pp_handle, long *p_autoreload);

/**
  * @brief  Get state of SW timer, similar to os_timer_is_timer_active() API.
  * @param[in]  pp_handle Pointer to the handle of timer
  * @param[out]  p_timer_state State of SW timer
  * @return Result of operation
  * \retval true  Get state of SW timer successfully.
  * \retval false Get state of SW timer failed.
  */
extern bool (*os_timer_state_get)(void **pp_handle, uint32_t *p_timer_state);

/** End of group OS_TIMER_Exported_Functions
  * @}
  */

/**
  * \cond     private
  * \defgroup OS_TIMER_Private_Functions
  * \{
  */

/**
  * @brief  Get timer number of SW timer.
  * @param[in]  pp_handle Pointer to the handle of timer
  * @param[out]  p_timer_number Number of SW timer
  * @return Result of operation
  * \retval true  Get SW timer number successfully.
  * \retval false Get SW timer number failed.
  */
extern bool (*os_timer_get_timer_number)(void **pp_handle, uint8_t *p_timer_number);

/**
  * @brief  Dump info for OS Timer.
  * @return Result of operation
  * \retval true  Get state of SW timer successfully.
  * \retval false Get state of SW timer failed.
  */
extern bool (*os_timer_dump)(void);

/**
  * @brief  OS Timer init.
  */
extern void (*os_timer_init)(void);
/**
  *  End of OS_TIMER_Private_Functions
  * \}
  * \endcond
  */

/** End of OS_TIMER
  * @}
  */


#ifdef __cplusplus
}
#endif

#endif /* _OS_TIMER_H_ */
