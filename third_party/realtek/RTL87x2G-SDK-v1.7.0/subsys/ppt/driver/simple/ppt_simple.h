/**
*****************************************************************************************
*     Copyright(c) 2021, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
  * @file     ppt_simple.h
  * @brief    Head file for simple wrapper of 2.4G module common driver.
  * @details  Data structs and external functions declaration.
  * @author   Bill
  * @date     2021-11-26
  * @version  v0.1
  * *************************************************************************************
  */

/* Define to prevent recursive inclusion */
#ifndef _PPT_SIMPLE_H_
#define _PPT_SIMPLE_H_

/* Add Includes here */
#include <stdbool.h>
#include <stdint.h>
#include "ppt_driver.h"

#ifdef  __cplusplus
extern "C" {
#endif      /* __cplusplus */

/** @addtogroup PPT_Simple
  * @{
  */

/** @defgroup PPT_Simple_Exported_Macros Exported Macros
  * @brief
  * @{
  */

#ifndef PPT_TX_BUFFER_NUM
/**
 * @brief Number of software buffers for tx packets which are the buffer RAM type.
 * All the data pushed to the TX FIFO shall be of the buffer RAM type.
 * When need more buffers, user can redefine this number using the compile parameter.
 */
#define PPT_TX_BUFFER_NUM           2
#endif
#define PPT_TX_BUFFER_SIZE          255 //!< Maximum length of the tx packet's payload.
#define PPT_RX_BUFFER_SIZE          (PPT_TX_BUFFER_SIZE + 8) //!< Maximum length of the rx packet including the header and the payload.

/** @} End of PPT_Simple_Exported_Macros */

/** @defgroup PPT_Simple_Exported_Types Exported Types
  * @brief
  * @{
  */

/** @brief The PTX mode extension parameters. */
typedef struct
{
    ppt_ptx_mode_t base; //!< The PTX mode parameters.
    uint16_t retransmit_times; //!< The ISR handler can uses the retransmit times to determine whether to disable PTX when using periodic mode.
} ppt_ptx_mode_ext_t;

/** @brief The PRX mode extension parameters. */
typedef struct
{
    ppt_prx_mode_t base; //!< The PRX mode parameters.
} ppt_prx_mode_ext_t;

/** @brief The PSD mode extension parameters. */
typedef struct
{
    ppt_psd_mode_t base; //!< The PSD mode parameters.
} ppt_psd_mode_ext_t;

typedef struct
{
    volatile ppt_fsm_t fsm; //!< Record current state.
    bool ack; //!< Ack mode.
    bool oneshot; //!< Oneshot mode.
    uint16_t retransmit_times; //!< Only used by the ptx, refer to @ref ppt_ptx_mode_ext_t.
    volatile uint16_t retransmit_counter; //!< Record current tx count.
    uint8_t *tx_buffer[PPT_TX_BUFFER_NUM]; //!< Tx packet buffers.
    uint8_t *rx_buffer; //!< Rx packet buffer.
    volatile bool sync_flag; //!< Flag indicates synchronous or asynchronous calling.
    void (*async_cb)(void); //!< Asynchronous callback when using asynchronous calling.
    struct
    {
        ppt_psd_mode_t psd_mode; //!< PSD mode.
        int16_t psd_result[PSD_CHANN_NUM]; //!< PSD result with unit of dBm.
        uint16_t psd_tmp_flag;  //!< Flag to restore an internal setting.
    };
} ppt_ctx_t;

extern ppt_ctx_t *ppt_ctx;
/** @} End of PPT_Simple_Exported_Types */

/** @defgroup PPT_Simple_Exported_Functions Exported Functions
  * @brief
  * @{
  */

/**
  * @brief Initialize the radio.
  */
void ppt_init(void);

/**
  * @brief Deinitialize the radio.
  */
void ppt_deinit(void);

/**
  * @brief Set the ptx modes.
  *
  * @param[in] param: PTX mode extension parameters.
  */
void ppt_set_ptx_mode_ext(ppt_ptx_mode_ext_t *param);

/**
  * @brief Set the prx modes.
  *
  * @param[in] param: PRX mode extension parameters.
  */
void ppt_set_prx_mode_ext(ppt_prx_mode_ext_t *param);

/**
  * @brief Set the psd modes.
  *
  * @param[in] param: PSD mode extension parameters.
  *
  * @return Result.
  * @retval True: Success.
  * @retval False: Channel start/stop/step settings are not valid.
  */
bool ppt_set_psd_mode_ext(ppt_psd_mode_ext_t *param);

/**
  * @brief Get the tx buffer used by the DMA for sending data.
  *
  * The RAM type used by the DMA is limited to a specific buffer RAM type.
  * This driver manages the memory for easy use at the application.
  *
  * @param[in] payload_len: Length of the payload.
  *
  * @return Pointer of the buffer.
  */
uint8_t *ppt_get_tx_buffer(uint16_t payload_len);

/**
  * @brief Get the tx buffer used by the DMA for sending data of designated channel.
  *
  * The RAM type used by the DMA is limited to a specific buffer RAM type.
  * This driver manages the memory for easy use at the application.
  *
  * @param[in] entry: Channel index.
  * @param[in] payload_len: Length of the payload.
  *
  * @return Pointer of the buffer.
  */
uint8_t *ppt_get_tx_buffer_by_entry(uint8_t entry, uint16_t payload_len);

/**
  * @brief Get the rx buffer used by the DMA for receiving data.
  *
  * The RAM type used by DMA is limited to a specific buffer RAM type.
  * This driver manages the memory for easy use at the application.
  *
  * @param[in] pdu_len: Length of the header and the payload.
  *
  * @return Pointer of the buffer.
  */
uint8_t *ppt_get_rx_buffer(uint16_t pdu_len);

/**
  * @brief Push and copy the data to the hw tx fifo of the channel 0.
  *
  * The data will be copied, so the RAM type of the data can be any type.
  *
  * @param[in] len: Length of the payload.
  * @param[in] data: Pointer of the payload.
  */
void ppt_push_tx_data(uint16_t len, uint8_t *data);

/**
  * @brief Update and copy the data to the hw tx fifo of the channel 0.
  *
  * The last pushed data can be updated by this function.
  * The channel index is 0.
  *
  * @param[in] len: Length of the payload.
  * @param[in] data: Pointer of the payload.
  */
void ppt_update_tx_data(uint16_t len, uint8_t *data);

/**
  * @brief Push and copy the data to the hw tx fifo of the specific channel.
  *
  * The data will be copied, so the RAM type of the data can be any type.
  *
  * @param[in] entry: Channel index.
  * @param[in] len: Length of the payload.
  * @param[in] data: Pointer of the payload.
  */
void ppt_push_tx_data_by_entry(uint8_t entry, uint16_t len, uint8_t *data);

/**
  * @brief Pop and copy the data from the hw rx fifo of the channel 0.
  *
  * The data will be copied to the buffer RAM whose pointer will be returned.
  *
  * @param[in] len: Length of the header and the payload.
  *
  * @return Pointer of the header and the payload.
  */
uint8_t *ppt_pop_rx_data(uint16_t len);

/**
  * @brief Pop and copy the data from the hw fifo of the specific channel.
  *
  * The data will be copied to the buffer RAM whose pointer will be returned.
  *
  * @param[in] entry: Channel index.
  * @param[in] len: Length of the header and the payload.
  *
  * @return Pointer of the header and the payload.
  */
uint8_t *ppt_pop_rx_data_by_entry(uint8_t entry, uint16_t len);

/**
  * @brief Asynchronous callback template.
  *
  * User can use this api for an asynchronous calling when there is no any operation at the asynchronous callback.
  */
void ppt_async_cb_template(void);

/**
  * @brief Enable ptx state.
  *
  * If the callback is null, then the api returns after the procedure is done in a synchronous way.
  * Otherwise, the caller will be notified via the callback when the procedure is done in an asynchronous way.
  *
  * @param[in] async_cb: Pointer of the asynchronous callback function.
  */
void ppt_enable_ptx(void (*async_cb)(void));

/**
  * @brief Disable ptx state.
  *
  * If the callback is null, then the API returns after the procedure is done in a synchronous way.
  * Otherwise, the caller will be notified via the callback when the procedure is done in an asynchronous way.
  *
  * @param[in] async_cb: Pointer of the asynchronous callback function.
  */
void ppt_disable_ptx(void (*async_cb)(void));

/**
  * @brief Enable prx state.
  *
  * If the callback is null, then the API returns after the procedure is done in a synchronous way.
  * Otherwise, the caller will be notified via the callback when the procedure is done in an asynchronous way.
  *
  * @param[in] async_cb: Pointer of the asynchronous callback function.
  */
void ppt_enable_prx(void (*async_cb)(void));

/**
  * @brief Disable prx state.
  *
  * If the callback is null, then the API returns after the procedure is done in a synchronous way.
  * Otherwise, the caller will be notified via the callback when the procedure is done in an asynchronous way.
  *
  * @param[in] async_cb: Pointer of the asynchronous callback function.
  */
void ppt_disable_prx(void (*async_cb)(void));

/**
  * @brief Reset the finite state machine of the hardware module.
  *
  * The reset feature shall be enabled previously by fw_rst_enable flag in @ref PRO_RESET_CTRL_REG.
  * This API will generate a mac interrupt, so it shall be called in the critical section if at lower priority.
  * All the internal states will be reset except for the interface registers @ref PPT_REG_NAME_ADDR.
  */
void ppt_reset_hw_fsm(void);

/**
  * @brief Kill the finite state machine of the hardware module.
  *
  * This API will generate a mac interrupt, so it shall be called in the critical section if at lower priority.
  *
  * @param[in] fsm: State to kill.
  *
  * @return Result.
  * @retval True: Success.
  * @retval False: Current state of the hardware mismatches the input state.
  */
bool ppt_kill_hw_fsm(ppt_fsm_t fsm);

/**
  * @brief Enable psd procedure.
  *
  * If the callback is null, then the API returns after the procedure is done in a synchronous way.
  * Otherwise, the caller will be notified via the callback when the procedure is done in an asynchronous way.
  *
  * @param[in] async_cb: Pointer of the asynchronous callback function.
  *
  * <b>Example usage</b>
  * \code{.c}
    ppt_psd_mode_ext_t param =
    {
        {
            .chann_start = 0,
            .chann_stop = 0,
            .chann_step = 1,
            .mode = 0,
            .timeout = PSD_TIMEOUT_DEFAULT
        }
    };
    ppt_set_psd_mode_ext(&param);
    ppt_enable_psd(NULL);
    int16_t rssi = ppt_get_psd_result(0);
  * \endcode
  */
void ppt_enable_psd(void (*async_cb)(void));

/**
  * @brief Disable psd procedure.
  *
  * If the callback is null, then the API returns after the procedure is done in a synchronous way.
  * Otherwise, the caller will be notified via the callback when the procedure is done in an asynchronous way.
  *
  * @param[in] async_cb: Pointer of the asynchronous callback function.
  */
void ppt_disable_psd(void (*async_cb)(void));

/**
  * @brief Get the psd result.
  *
  * @param[in] chann: Channel index, shall be less than @ref PSD_CHANN_NUM.
  *
  * @return Channel power spectrum density in dBm.
  */
int16_t ppt_get_psd_result(uint8_t chann);

/**
  * @brief Clear the psd results.
  */
void ppt_clear_psd_result(void);

/** @} End of PPT_Simple_Exported_Functions */

/** @} End of PPT_Simple */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
