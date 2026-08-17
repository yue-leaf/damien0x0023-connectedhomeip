/**
*****************************************************************************************
*     Copyright(c) 2020, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
  * @file     ppt_driver.h
  * @brief    Head file for 2.4G module common driver.
  * @details  Data structs and external functions declaration.
  * @author   Bill
  * @date     2020-12-03
  * @version  v0.1
  * *************************************************************************************
  */

/* Define to prevent recursive inclusion */
#ifndef _PPT_DRIVER_H_
#define _PPT_DRIVER_H_

/* Add Includes here */
#include <stdbool.h>
#include <stdint.h>
#include "app_section.h"
#include "ppt_hw_reg.h"
#include "ll_common.h"
#include "ppt_pf.h"

#ifdef  __cplusplus
extern "C" {
#endif      /* __cplusplus */

/** @addtogroup PPT_Driver
  * @{
  */

/** @defgroup PPT_Driver_Exported_Macros Exported Macros
  * @brief
  * @{
  */

/** @defgroup PPT_SECTION       Section
  * @brief Control the memory location of functions & data.
  * The processing can speed up if a function is placed at the RAM region.
  * @{
  */
#define PPT_ISR_SECTION             RAM_FUNCTION //!< Interrupt handler section.
#define PPT_API_SECTION             RAM_FUNCTION //!< Common api section.
#define PPT_DAT_SECTION             //!< Data section.
/** @} */

/** @defgroup PPT_Hardware_Channels     Hardware Channels
  * @brief Hardware multiple channels trx related macros used to access registers & check parameter.
  * @{
  */
#define PPT_ENTRY_NUM               8 //!< Total channel number.
#define PPT_ENTRY_NUM_ORI           3 //!< Channel number for old design.
#define PPT_ENTRY_RX_NUM            8 //!< Total rx channel number.
#define PPT_ENTRY_RX_NUM_ORI        2 //!< Rx channel number for old design.
/** @} */

/** @defgroup PPT_HW_MISC  Hardware Misc
  * @brief Hardware miscellaneous definition.
  * @{
  */
#define PPT_ADDR_LEN_MAX            5 //!< Maximum length of the access address supported by the hardware.
#define PPT_TX_FIFO_SIZE            8 //!< Tx FIFO depth supported by the hardware.

#define PRO_ONE_ENTRY_DW_SIZE       0x20 //!< Macro used for the convenience of accessing registers.
#define PRO_ENTRY_FIELD_BASE(entry) ((entry) * PRO_ONE_ENTRY_DW_SIZE) //!< Macro used for the convenience of accessing registers.
#define PRO_ONE_PREFIX_DW_SIZE      0x2 //!< Macro used for the convenience of accessing registers.
#define PRO_PREFIX_BASE(entry)      ((entry) * PRO_ONE_PREFIX_DW_SIZE) //!< Macro used for the convenience of accessing registers.

#define PPT_ISR_NUMBER              7 //!< ISR type number.
#define PPT_ISR_VALID_BITS_MASK     0x7F //!< ISR type bit mask.

#define PPT_NATIVE_TIME_MAX_VALUE   (83886080000 - 1) //!< Native time timer maximum count value. Refer to @ref ppt_native_time_elapse.
/** @} */

/** @defgroup PPT_REG_ACCESS     Register Access
  * @brief Macros used to access radio registers. All radio registers are 16 bits wide.
  * @{
  */

/**
  * @brief Read the radio register.
  *
  * @param[in] offset: Register address offset, refer to @ref PPT_REG_NAME_ADDR.
  *
  * @return Register value.
  */
#define RD_PPT_REG(offset) ppt_read_reg(offset)

/**
  * @brief Write the radio register.
  *
  * @param[in] offset: Register address offset, refer to @ref PPT_REG_NAME_ADDR.
  * @param[in] data: Register value.
  */
#define WR_PPT_REG(offset, data) ppt_write_reg(offset, data)

/**
  * @brief Update only the masked bits of the radio register.
  *
  * @param[in] offset: Register address offset, refer to @ref PPT_REG_NAME_ADDR.
  * @param[in] mask: Mask of bits to be modified.
  * @param[in] data: Register value.
  */
#define UP_PPT_REG(offset, mask, data) do\
    {\
        uint16_t reg = RD_PPT_REG(offset);\
        reg &= ~(mask);\
        reg |= (data) & (mask);\
        WR_PPT_REG(offset, reg);\
    } while(0)

/**
  * @brief Overwrite only the specific range of bits of the radio register.
  *
  * @param[in] offset: Register address offset, refer to @ref PPT_REG_NAME_ADDR.
  * @param[in] field_offset: The field bit offset.
  * @param[in] field_len: The field bit length.
  * @param[in] data: The field value.
  */
#define OV_PPT_REG_FIELD(offset, field_offset, field_len, data) do\
    {\
        uint16_t reg = RD_PPT_REG(offset);\
        reg &= ~(((0xffff >> (16 - (field_len)))) << (field_offset));\
        reg |= (data) << (field_offset);\
        WR_PPT_REG(offset, reg);\
    } while(0)

/**
  * @brief Read the specific field of the radio register.
  *
  * @param[in] name: Register name, refer to @ref PPT_REG_NAME_ADDR.
  * @param[in] field: The specific field name.
  *
  * @return The specific field value.
  */
#define RD_PPT_REG_FIELD(name, field) ({name##_TYPE reg = {.d16 = RD_PPT_REG(name)}; reg.field;})

/**
  * @brief Write the specific field of the radio register.
  *
  * @param[in] name: Register name, refer to @ref PPT_REG_NAME_ADDR.
  * @param[in] field: The specific field name.
  * @param[in] data: The specific field value.
  */
#define WR_PPT_REG_FIELD(name, field, data) do\
    {\
        name##_TYPE reg = {.d16 = RD_PPT_REG(name)};\
        reg.field = data;\
        WR_PPT_REG(name, reg.d16);\
    } while (0)

/**
  * @brief Update only the masked bits of the specific field of the radio register.
  *
  * @param[in] name: Register name, refer to @ref PPT_REG_NAME_ADDR.
  * @param[in] field: The specific field name.
  * @param[in] mask: Mask of bits to be modified.
  * @param[in] data: The specific field value.
  */
#define UP_PPT_REG_FIELD(name, field, mask, data) do\
    {\
        name##_TYPE reg = {.d16 = RD_PPT_REG(name)};\
        reg.field &= ~(mask);\
        reg.field |= (data) & (mask);\
        WR_PPT_REG(name, reg.d16);\
    } while(0)
/** @} */

/** @defgroup PPT_SW_MISC  Software Misc
  * @brief Software miscellaneous definition.
  * @{
  */

/**
  * @brief Obtain the register offset of the specific entry.
  *
  * Some registers have specific offsets at different entry to facilitate obtaining the register address offset for the specific entry.
  *
  * @param[in] entry: Channel index.
  * @param[in] offset0: Register offset0.
  * @param[in] size0: Entry step size0.
  * @param[in] offset1: Register offset1.
  * @param[in] size1: Entry step size1.
  *
  * @return Register address offset.
  */
#define PPT_REG_OFFSET(entry, offset0, size0, offset1, size1) (((entry) < PPT_ENTRY_NUM_ORI) ? ((size0)*(entry) + (offset0)) : ((size1) * ((entry) - PPT_ENTRY_NUM_ORI) + (offset1)))

/**
  * @brief Obtain the tx entry in the tx stack information.
  *
  * The tx entry value is contained in @ref PRO_TX_STACK and @ref PRO_TX_LENGTH two registers.
  *
  * @param[in] tx_stack: Refer to @ref PRO_TX_STACK_TYPE.
  * @param[in] tx_len: Refer to @ref PRO_TX_LENGTH_TYPE.
  *
  * @return The tx entry in the tx stack information.
  */
#define PPT_TX_STACK_ENTRY(tx_stack, tx_len) (tx_stack.tx_entry_1_0 + (tx_len.tx_entry_2 << 2))

/**
  * @brief Obtain the rx entry in the rx stack information.
  *
  * The rx entry value is contained in two discontinuous fields of @ref PRO_RX_STACK register.
  *
  * @param[in] rx_stack: Refer to @ref PRO_RX_STACK_TYPE.
  *
  * @return The rx entry in the rx stack information.
  */
#define PPT_RX_STACK_ENTRY(rx_stack) (rx_stack.entry_0 + (rx_stack.entry_2_1 << 1))

/**
  * @brief Obtain the header size pushed to the Rx FIFO.
  *
  * The Rx FIFO is filled with the header and the payload of a packet.
  * The header contains the header prefix, length and header suffix.
  * The total size will be aligned to 16 bits when filling to the Rx FIFO.
  *
  * @param[in] hp_size: Header prefix size in bits.
  * @param[in] length_size: Payload length size in bits.
  * @param[in] hs_size: Header suffix size in bits.
  *
  * @return Header size in bytes.
  */
#define PPT_RX_FIFO_HEADER_SIZE(hp_size, length_size, hs_size) ((((hp_size) + (length_size) + (hs_size) + 15) >> 4) << 1)

/**
  * @brief Obtain the header size pushed to the Rx FIFO.
  *
  * The Rx FIFO is filled with the header and the payload of a packet.
  * The header contains the header prefix, length and header suffix.
  * The total size will be aligned to 16 bits when filling to the Rx FIFO.
  *
  * @param[in] hp_size: Header prefix size in bytes.
  * @param[in] length_size: Payload length size in bytes.
  * @param[in] hs_size: Header suffix size in bytes.
  *
  * @return Header size in bytes.
  */
#define PPT_RX_FIFO_HEADER_SIZE_BYTE(hp_size, length_size, hs_size) PPT_RX_FIFO_HEADER_SIZE(hp_size << 3, length_size << 3, hs_size << 3)

/**
  * @brief Enable radio interrupt.
  */
#define PPT_ISR_ENABLE()        NVIC_EnableIRQ(BTMAC_IRQn)

/**
  * @brief Disable radio interrupt.
  */
#define PPT_ISR_DISABLE()       NVIC_DisableIRQ(BTMAC_IRQn)

/**
  * @brief Calculate elapsed time of two native time timestamps.
  *
  * Use @ref ppt_native_time_read to attain the native time.
  *
  * @param[in] time_end: Time ending point.
  * @param[in] time_start: Time starting point.
  *
  * @return Elapsed time in microsecond.
  */
#define ppt_native_time_elapse(time_end, time_start)    ((time_end) >= (time_start) ? (time_end) - (time_start) : PPT_NATIVE_TIME_MAX_VALUE - (time_start) + 1 + (time_end))
/** @} */

/** @} End of PPT_Driver_Exported_Macros */

/** @defgroup PPT_Driver_Exported_Types Exported Types
  * @brief
  * @{
  */

/** @brief Hw supported instructions. */
typedef enum
{
    PPT_HW_INSTRUCTION_PTX_ENABLE           = 0, //!< Enable PTX instruction.
    PPT_HW_INSTRUCTION_PRX_ENABLE           = 1, //!< Enable PRX instruction.
    PPT_HW_INSTRUCTION_PTX_DISABLE          = 2, //!< Disable PTX instruction.
    PPT_HW_INSTRUCTION_PRX_DISABLE          = 3, //!< Disable PRX instruction.
    PPT_HW_INSTRUCTION_PSD_ENABLE           = 4, //!< Enable PSD instruction.
    PPT_HW_INSTRUCTION_PSD_DISABLE          = 5, //!< Disable PSD instruction.
    PPT_HW_INSTRUCTION_MAX  //!< Instruction maximum number.
} ppt_hw_instruction_t;

/** @brief Hw supported states. */
typedef enum
{
    PPT_FSM_STANDBY, //!< Standby state.
    PPT_FSM_PTX, //!< PTX state.
    PPT_FSM_PRX, //!< PRX state.
    PPT_FSM_PSD //!< PSD state.
} ppt_fsm_t;

/** @brief The bit endianess of packet field. */
typedef enum
{
    PPT_FIELD_BIT_ORDER_LSB, //!< Least significant bit first.
    PPT_FIELD_BIT_ORDER_MSB //!< Most significant bit first.
} ppt_field_bit_order_t;

/** @brief The packet format control parameters. */
typedef struct
{
    uint8_t addr_len; //!< Length of access address in bytes.
    uint8_t hp_len; //!< Length of header prefix in bits.
    uint8_t length_len; //!< Length of payload length in bits.
    uint8_t hs_len; //!< Length of header suffix in bits.
    ppt_field_bit_order_t header_order;  //!< Bit endianess of header.
    ppt_field_bit_order_t payload_order; //!< Bit endianess of payload.
} ppt_pkt_format_t;

/** @brief The addon field length parameters. */
typedef struct
{
    uint8_t len; //!< Length of addon field in bytes.
} ppt_addon_length_t;

/** @brief The CRC formula parameters. */
typedef struct
{
    uint32_t poly; //!< CRC polynomial.
    uint32_t init; //!< CRC initial value.
} ppt_crc_value_t;

/** @brief The CRC length parameters for all channels. */
typedef struct
{
    uint8_t len; //!< Length of CRC in bytes.
    bool include_addr; //!< Calculation includes access address. Not supported feature!
} ppt_crc_param_t;

/** @brief CRC parameters of each channel. */
typedef struct
{
    ppt_crc_value_t value; //!< Crc formula parameters.
} ppt_crc_entry_param_t;

/** @brief The whitening formula parameters. */
typedef struct
{
    uint32_t poly; //!< Whitening polynomial.
    uint32_t init; //!< Whitening initial value.
} ppt_white_value_t;

/** @brief The whitening switch for all channels. */
typedef struct
{
    bool enable;
} ppt_white_param_t;

/** @brief The whitening parameters of each channel. */
typedef struct
{
    uint8_t len; //!< Length of whitening coding in bits.
    ppt_white_value_t value; //!< Whitening formula parameters
} ppt_white_entry_param_t;

/** @brief Supported PHY types. */
typedef enum
{
    PPT_PHY_TYPE_BLE_1M, //!< BLE 1 Mbps phy type.
    PPT_PHY_TYPE_BLE_2M //!< BLE 2 Mbps phy type.
} ppt_phy_type_t;

/** @brief The PHY common parameters for all channels. */
typedef struct
{
    uint8_t bank; //!< RF frequency bank.
    uint8_t channel;  //!< RF frequency channel.
    ppt_phy_type_t rx_phy; //!< PHY types of RX.
} ppt_phy_param_t;

/** @brief The PHY special parameters of each channel. */
typedef struct
{
    uint8_t preamble_len; //!< Length of preamble field.
    ppt_phy_type_t tx_phy; //!< PHY types of TX.
} ppt_phy_entry_param_t;

/** @brief The tx access address of each channel. */
typedef struct
{
    uint8_t tx_addr[PPT_ADDR_LEN_MAX]; //!< TX access address.
} ppt_tx_addr_t;

/** @brief The rx access address of each channel. */
typedef struct
{
    bool enable; //!< Enable or disable RX for this channel.
    uint8_t rx_addr[PPT_ADDR_LEN_MAX]; //!< RX access address.
} ppt_rx_addr_t;

/** @brief The header fields of frame of each channel. */
typedef struct
{
    uint8_t hp; //!< Header prefix field value.
    uint16_t length; //!< Payload length field value. This value is set by the DMA automatically.
    uint32_t hs; //!< Header suffix field value.
} ppt_header_t;

/** @brief The PTX mode parameters for all channels. */
typedef struct
{
    bool ack_mode; //!< Enable or disable the ack mode.
    bool periodic_mode; //!< Enable or disable the periodic mode.
    uint32_t periodic_interval; //!< Interval with 20 valid bits for the periodic mode, unit is 125us, interval = (n+1)*125us.
} ppt_ptx_mode_t;

/** @brief The PRX mode parameters for all channels. */
typedef struct
{
    bool ack_mode; //!< Enable or disable the ack mode.
    bool continuous_mode; //!< Enable or disable the continuous mode.
} ppt_prx_mode_t;

/** @brief The PSD mode parameters. */
typedef struct
{
    uint8_t chann_start; //!< Starting point of the RF frequency channel.
    uint8_t chann_stop; //!< Stopping point of the RF frequency channel.
    uint8_t chann_step; //!< RF frequency channel step for next round psd. Next channel = current channel + channel step.
    uint8_t mode; //!< Useless.
    uint16_t timeout; //!< PSD timeout setting in microsends. Maximum value @ref PSD_TIMEOUT_DEFAULT is recommended.
} ppt_psd_mode_t;

/** @brief The timing latch parameters for attaining the timing of trx. */
typedef struct
{
    bool tx_on; //!< Enable or disable timing latch when the tx on event happens at tx.
    bool acc_hit; //!< Enable or disable timing latch when the access address hit event happens at rx.
} ppt_timing_latch_t;

/** @brief The GPIO trigger PTX parameters of each channel. */
typedef struct
{
    uint16_t gpio_delay; //!< Delay from the input GPIO signal to the enable PTX instruction is auto executed. Delay = (gpio_delay + 1) * 25us.
} ppt_gpio_entry_param_t;

/** @brief The GPIO trigger PTX common parameters for all channels. */
typedef struct
{
    bool enable; //!< Enable or disable GPIO trigger PTX feature.
    bool int_enable; //!< Enable or disable gpio interrupt.
} ppt_gpio_param_t;

/** @brief The CRC engine common parameters. */
typedef struct
{
    uint32_t poly; //!< CRC polynomial.
    uint32_t init; //!< CRC initial value.
    uint8_t length; //!< Length of CRC result in bits
    uint8_t input_endian : 1; //!< Input endianess, 0: lsb, 1: msb.
    uint8_t output_endian : 1; //!< Output endianess, 0: lsb, 1: msb.
    uint8_t bit_mode : 1; //!< Byte or bit mode choosen, 0: byte, 1: bit.
} ppt_crc_eng_param_t;

/** @brief Hw register field descriptor. */
typedef struct
{
    uint16_t reg_offset; //!< Register address offset, refer to @ref PPT_REG_NAME_ADDR.
    uint8_t field_offset; //!< Register field bit offset.
} ppt_reg_field_t;

/** @brief Flag for user to tell driver whether it can sleep. */
extern volatile bool ppt_dlps_mac_idle;

/** @} End of PPT_Driver_Exported_Types */

/** @defgroup PPT_Driver_Exported_Functions Exported Functions
  * @brief
  * @{
  */

/**
  * @brief Read the radio register.
  *
  * @param[in] offset: Register address offset, refer to @ref PPT_REG_NAME_ADDR.
  *
  * @return Register value.
  */
extern uint16_t (*ppt_read_reg)(uint16_t offset);

/**
  * @brief Write the radio register.
  *
  * @param[in] offset: Register address offset, refer to @ref PPT_REG_NAME_ADDR.
  * @param[in] data: Register value.
  */
extern void (*ppt_write_reg)(uint16_t offset, uint16_t data);

/**
  * @brief Swap bit order of one byte data.
  *
  * @param[in] data: The data to be swapped.
  *
  * @return The new value after swapped bit order
  */
uint8_t ppt_swap_bits8(uint8_t data);

/**
  * @brief Swap bit order of two bytes data.
  *
  * @param[in] data: The data to be swapped.
  *
  * @return The new value after swapped bit order.
  */
uint16_t ppt_swap_bits16(uint16_t data);

/**
  * @brief Swap bit order of three bytes data.
  *
  * @param[in] data: The data to be swapped.
  *
  * @return The new value after swapped bit order.
  */
uint32_t ppt_swap_bits24(uint32_t data);

/**
  * @brief Swap bit order of four bytes data.
  *
  * @param[in] data: The data to be swapped.
  *
  * @return The new value after swapped bit order.
  */
uint32_t ppt_swap_bits32(uint32_t data);

/**
  * @brief Initialize the DMA of the radio.
  */
void ppt_init_dma(void);

/**
  * @brief Workaround for reset when the DMA starts which results in dma carry error later.
  */
void ppt_clear_dma_tx_done(void);

/**
  * @brief Deinitialize the DMA of the radio.
  */
void ppt_deinit_dma(void);

/**
  * @brief Initialize the miscellaneous functions.
  */
void ppt_init_misc(void);

/**
  * @brief Deinitialize the miscellaneous functions.
  */
void ppt_deinit_misc(void);

/**
  * @brief Reset trx packet number counter of hardware.
  */
void ppt_reset_trx_pkt_num(void);

/**
  * @brief Set the clock of radio.
  *
  * @param[in] enable: Enable or disable the clock.
  */
void ppt_reset_core_domain(bool enable);

/**
  * @brief Notify the BT MAC of the 2.4G radio on-off state to optimize the performance.
  *
  * The BT MAC may optimize the power consumption etc.
  * @param[in] enable: Enable or disable state of the 2.4G radio.
  */
void ppt_notify_btmac(bool enable);

/**
  * @brief Execute instruction to switch the radio state.
  *
  * @param[in] instruction_code: Instruction opcode @ref ppt_hw_instruction_t.
  */
void ppt_execute_instruction(uint8_t instruction_code);

/**
  * @brief Clear radio state.
  *
  * @deprecated Not used.
  */
#define ppt_clear_radio()

/**
  * @brief Clear radio state.
  *
  * @deprecated Not used.
  */
#define ppt_clear_radio_quick(...)

/**
  * @brief Lock/unlock the header fields value.
  *
  * Since the header has multiple fields, they can be written in an atomic way after lock.
  *
  * @param[in] entry: Channel index.
  * @param[in] lock: True for lock, false for unlock.
  */
void ppt_lock(uint8_t entry, bool lock);

/**
  * @brief Set whether to hold tx automatically by the hardware.
  *
  * @param[in] entry: Channel index.
  * @param[in] enable: True for enable, false for disable.
  */
void ppt_set_auto_no_tx(uint8_t entry, bool enable);

/**
  * @brief Clear hardware automatically holding tx once.
  *
  * @param[in] entry: Channel index.
  */
void ppt_clear_hw_no_tx(uint8_t entry);

/**
  * @brief Set the feature of hardware auto ack the transmitted packet.
  *
  * When auto ack is enabled, the read pointer of the TX FIFO will automatically add one
  * after the packet is sent. It means the transmitted packet is discarded automatically.
  *
  * @param[in] entry: Channel index.
  * @param[in] enable: True for enable, false for disable.
  */
void ppt_set_auto_ack(uint8_t entry, bool enable);

/**
  * @brief Firmware ack the transmitted packet once with lock flag.
  *
  * The read pointer of the TX FIFO will add one which means the transmitted packet is discarded.
  *
  * @param[in] entry: Channel index.
  * @param[in] lock: True for lock the header and the TX FIFO.
  *
  * @return Read pointer of the TX FIFO.
  */
uint8_t ppt_trigger_fw_ack_lock(uint8_t entry, bool lock);

/**
  * @brief Firmware ack the transmitted packet once.
  *
  * The read pointer of the TX FIFO will add one which means the transmitted packet is discarded.
  *
  * @param[in] entry: Channel index.
  *
  * @return Read pointer of the TX FIFO.
  */
#define ppt_trigger_fw_ack(entry) ppt_trigger_fw_ack_lock(entry, true)

/**
  * @brief Firmware force ack the first packet which may not be transmitted.
  *
  * If the first packet in the TX FIFO is not transmitted,
  * use @ref ppt_trigger_fw_ack to ack the packet is not working.
  * Instead, use ppt_force_fw_ack to force ack to discard the packet.
  * PRX role may use this api to quit data pushed to the TX FIFO previously.
  *
  * @param[in] entry: Channel index.
  *
  * @return Read pointer of the TX FIFO.
  */
uint8_t ppt_force_fw_ack(uint8_t entry);

/**
  * @brief Register the interrupt handler.
  *
  * Except for the PSD interrupt, all types of radio interrupts are handled at here.
  *
  * @param[in] handler: Function pointer of ISR handler.
  */
void ppt_reg_handler(void (*handler)(void));

/**
  * @brief Get the registered interrupt handler.
  *
  * @return The interrupt handler.
  */
void (*ppt_get_handler(void))(void);

/**
  * @brief Register the psd interrupt handler.
  *
  * @param[in] handler: Function pointer of PSD ISR handler.
  */
void ppt_reg_psd_handler(void (*handler)(void));

/**
  * @brief Set CRC common parameters for all channels.
  *
  * @note CRC including address field is not supported.
  * It can be as a workaround to replace the origin CRC init value by a new CRC init value
  * which is calculated through the address address as the CRC input data using the origin CRC init.
  * Only replace the CRC init value, but the CRC polynomial is not changed all the time.
  *
  * @param[in] param: CRC common parameters
  */
void ppt_set_crc_param(ppt_crc_param_t *param);

/**
  * @brief Set CRC specific parameters of each channel.
  *
  * @param[in] entry: Channel index.
  * @param[in] param: CRC specific parameters.
  */
void ppt_set_crc_entry_param(uint8_t entry, ppt_crc_entry_param_t *param);

/**
  * @brief Set whitening common parameters for all channels.
  *
  * @param[in] param: Whitening common parameters.
  */
void ppt_set_white_param(ppt_white_param_t *param);

/**
  * @brief Set whitening specific parameters of each channel.
  *
  * @param[in] entry: Channel index.
  * @param[in] param: Whitening specific parameters.
  */
void ppt_set_white_entry_param(uint8_t entry, ppt_white_entry_param_t *param);

/**
  * @brief Set rx phy for all channels.
  *
  * @param[in] rx_phy: RX phy type.
  */
void ppt_set_phy_rx_type(ppt_phy_type_t rx_phy);

/**
  * @brief Set tx phy of each channel.
  *
  * @param[in] entry: Channel index.
  * @param[in] tx_phy: TX phy type.
  */
void ppt_set_phy_tx_type(uint8_t entry, ppt_phy_type_t tx_phy);

/**
  * @brief Set phy common parameters for all channels.
  *
  * @param[in] param: Phy common parameters.
  */
void ppt_set_phy_param(ppt_phy_param_t *param);

/**
  * @brief Set phy specific parameters of each channel.
  *
  * @param[in] entry: Channel index.
  * @param[in] param: Phy specific parameters.
  */
void ppt_set_phy_entry_param(uint8_t entry, ppt_phy_entry_param_t *param);

/**
  * @brief Set modulation index of 2M phy.
  *
  * @param[in] change: True for change from the default value 0.5 to 0.32.
  */
void ppt_set_phy_mod_index_2m(bool change);

/**
  * @brief Set phy parameters for different rf bank.
  *
  * Currently only modulation tx parameters are updated by this function.
  * So only tx needs to call this function to apply new settings.
  *
  * @param[in] bank_idx: 0 for 2402~2480MHz.
  */
void ppt_set_phy_bank(uint8_t bank_idx);

/**
  * @brief Set phy rf channel.
  *
  * It will call ppt_set_phy_bank to update phy parameters at the same time.
  *
  * @param[in] freq_mhz: Frequency in MHz, range 2402~2480MHz.
  *
  * @return Set result.
  * @retval True: Success.
  * @retval False: Failure since the frequency is not supported.
  */
bool ppt_set_phy_channel(uint16_t freq_mhz);

/**
  * @brief Set packet format parameters.
  *
  * @param[in] param: Packet format parameters.
  */
void ppt_set_pkt_format(ppt_pkt_format_t *param);

/**
  * @brief Set preamble length of each channel.
  *
  * @param[in] entry: Channel index.
  * @param[in] preamble_len: Length in bytes.
  */
void ppt_set_preamble_len(uint8_t entry, uint8_t preamble_len);

/**
  * @brief Set addon field parameters of each channel.
  *
  * @param[in] entry: Channel index.
  * @param[in] param: Addon field parameters.
  */
void ppt_set_addon_len(uint8_t entry, ppt_addon_length_t *param);

/**
  * @brief Set rf tx power via tx gain.
  *
  * @param[in] tx_gain: Different tx gain values correspond to diffent tx powers.
  */
void ppt_set_tx_power(uint8_t tx_gain);

/**
  * @brief Set rf tx power in dBm.
  *
  * @param[in] tx_power_dbm: Tx power in dBm.
  */
void ppt_set_tx_power_dbm(float tx_power_dbm);

/**
  * @brief Set tx access address of each channel.
  *
  * @param[in] entry: Channel index.
  * @param[in] param: Tx access address.
  */
void ppt_set_tx_addr(uint8_t entry, ppt_tx_addr_t *param);

/**
  * @brief Set rx access address of each channel.
  *
  * The caller shall check whether any address value is 0xff or two rx addresses values are similar.
  * The access address shall be chaotic as the definition in BT specification.
  * If not, the radio parameter needs to be changed to receive the packets successfully.
  * But this will decrease the rx sensitivity.
  *
  * The radio parameter can be updated by @ref ppt_check_rx_addr.
  *
  * @param[in] entry: Channel index.
  * @param[in] param: Rx access address.
  */
void ppt_set_rx_addr(uint8_t entry, ppt_rx_addr_t *param);

/**
  * @brief Check rx access address and set radio parameters.
  *
  * If any address value is 0xff or two rx addresses values are similar, this function will set the radio parameters.
  * Since the parameters are set according to the chosen rx PHY, this function shall be called
  * after both the address and rx PHY are set.
  *
  * @param[in] bit_th: Bit difference threshold, recommended to use 5.
  */
void ppt_check_rx_addr(uint8_t bit_th);

/**
  * @brief Set ptx modes.
  *
  * @param[in] param: PTX mode parameters.
  */
void ppt_set_ptx_mode(ppt_ptx_mode_t *param);

/**
  * @brief Set prx modes.
  *
  * @param[in] param: PRX mode parameters.
  */
void ppt_set_prx_mode(ppt_prx_mode_t *param);

/**
  * @brief Set psd modes.
  *
  * @param[in] param: PSD mode parameters.
  */
void ppt_set_psd_mode(ppt_psd_mode_t *param);

/**
  * @brief Set header prefix field of tx packet of each channel.
  *
  * @param[in] entry: Channel index.
  * @param[in] hp: Header prefix value.
  */
void ppt_set_hp(uint8_t entry, uint8_t hp);

/**
  * @brief Set payload length field of tx packet of each channel.
  *
  * The value of the length field indicates the octet length of the payload.
  * But this value is useless if dma_length_mode is enabled (which is default on) which uses dma length instead.
  *
  * @param[in] entry: Channel index.
  * @param[in] length: Payload length.
  */
void ppt_set_length(uint8_t entry, uint16_t length);

/**
  * @brief Set header suffix field of tx packet of each channel.
  *
  * @param[in] entry: Channel index.
  * @param[in] hs: Header suffix value.
  */
void ppt_set_hs(uint8_t entry, uint32_t hs);

/**
  * @brief Set header field value of tx packet of each channel.
  * @param[in] entry: Channel index.
  * @param[in] param: Header field value.
  */
void ppt_set_tx_header(uint8_t entry, ppt_header_t *param);

/**
  * @brief Set rf turn around time.
  *
  * If not set, the default value is 150us.
  *
  * @param[in] tifs_us: TIFS value in microsecond.
  */
void ppt_set_tifs(uint8_t tifs_us);

/**
  * @brief Set timing latch parameters.
  *
  * @param[in] param: Timing latch parameters.
  */
void ppt_set_timing_latch(ppt_timing_latch_t *param);

/**
  * @brief Set GPIO trigger PTX common parameters for all channels.
  *
  * @param[in] param: GPIO trigger PTX common parameters.
  */
void ppt_set_gpio_param(ppt_gpio_param_t *param);

/**
  * @brief Set GPIO trigger channel specific parameters of each channel.
  *
  * @param[in] entry: Channel index.
  * @param[in] param: Channel specific parameters.
  */
void ppt_set_gpio_entry_param(uint8_t entry, ppt_gpio_entry_param_t *param);

/**
  * @brief Push the payload field to the TX FIFO.
  *
  * The hardware won't copy the data, so the data needs to be valid until it is acknowledged.
  * The other fields will be filled to the transmitted frame automatically by the hardware.
  *
  * @param[in] entry: Channel index.
  * @param[in] payload: Pointer of payload.
  * @param[in] payload_len: Length of payload.
  *
  * @return Push result.
  * @retval True: Success.
  * @retval False: DMA error.
  */
bool ppt_push_tx_fifo(uint8_t entry, uint8_t *payload, uint16_t payload_len);

/**
  * @brief Update the payload field to the TX FIFO.
  *
  * The hardware won't copy the data, so the data needs to be valid until it is acknowledged.
  * The other fields will be filled to the transmitted frame automatically by the hardware.
  *
  * @param[in] entry: Channel index.
  * @param[in] payload: Pointer of payload.
  * @param[in] payload_len: Length of payload.
  *
  * @return Push result.
  * @retval True: Success.
  * @retval False: DMA error.
  */
bool ppt_update_tx_fifo(uint8_t entry, uint8_t *payload, uint16_t payload_len);

/**
  * @brief Flush the TX FIFO.
  *
  * @param[in] entry: Channel index.
  *
  * @return Read pointer of the TX FIFO.
  */
uint8_t ppt_flush_tx_fifo(uint8_t entry);

/**
  * @brief Get the read pointer of the TX FIFO.
  *
  * @param[in] entry: Channel index.
  *
  * @return Read pointer of the TX FIFO.
  */
uint8_t ppt_get_tx_fifo_rdptr(uint8_t entry);

/**
  * @brief Set the read pointer of the TX FIFO.
  *
  * @param[in] entry: Channel index.
  * @param[in] ptr: Read pointer where to start tx next time.
  */
void ppt_set_tx_fifo_rdptr(uint8_t entry, uint8_t ptr);

/**
  * @brief Get the write pointer of the TX FIFO.
  *
  * @param[in] entry: Channel index.
  *
  * @return Write pointer of the TX FIFO.
  */
uint8_t ppt_get_tx_fifo_wrptr(uint8_t entry);

/**
  * @brief Get the pending packet number of the TX FIFO.
  *
  * @param[in] entry: Channel index.
  *
  * @return Number of packets pending at the TX FIFO.
  */
uint8_t ppt_get_tx_fifo_pend_pkt_num(uint8_t entry);

/**
  * @brief Get the free packet number of the TX FIFO.
  *
  * @param[in] entry: Channel index.
  *
  * @return Number of packets that can be sent to the TX FIFO.
  */
uint8_t ppt_get_tx_fifo_free_pkt_num(uint8_t entry);

/**
  * @brief Reset the TX FIFO sw & hw state.
  *
  * The read and write pointers will be set to zero, and the fifo will be empty.
  *
  * @param[in] entry: Channel index.
  */
void ppt_reset_tx_fifo(uint8_t entry);

/**
  * @brief Pop the received PDU from the rx fifo.
  *
  * The rx fifo contains the PDU data including the header fields and payload field.
  *
  * The length of the PDU is the length of payload plus the length of header.
  * The length of payload is attained at the rx stack information, refer to @PRO_LENGTH_TYPE.
  * The length of header can be calculated by @ref PPT_RX_FIFO_HEADER_SIZE or @ref PPT_RX_FIFO_HEADER_SIZE_BYTE.
  *
  * @param[in] entry: Channel index.
  * @param[in] pdu: Pointer of PDU.
  * @param[in] pdu_len: Length of PDU.
  *
  * @return Operation result.
  * @retval True: Success.
  * @retval False: DMA error.
  */
bool ppt_pop_rx_fifo(uint8_t entry, uint8_t *pdu, uint16_t pdu_len);

/**
  * @brief Pop the received pdu from the rx fifo with wait option.
  *
  * The rx fifo contains the PDU data including the header fields and payload field.
  *
  * The length of the PDU is the length of payload plus the length of header.
  * The length of payload is attained at the rx stack information, refer to @PRO_LENGTH_TYPE.
  * The length of header can be calculated by @ref PPT_RX_FIFO_HEADER_SIZE or @ref PPT_RX_FIFO_HEADER_SIZE_BYTE.
  *
  * When choosing to wait, this api equals @ref ppt_pop_rx_fifo.
  * When choosing to not wait, this api returns without waiting for the DMA move operation to be done.
  * The user can use @ref ppt_pop_rx_fifo_wait_done to check & wait for the DMA operation later.
  *
  * @param[in] entry: Channel index.
  * @param[in] pdu: Pointer of PDU.
  * @param[in] pdu_len: Length of PDU.
  * @param[in] wait: Choose whether to wait for DMA move to be done.
  *
  * @return Operation result.
  * @retval True: Success.
  * @retval False: DMA error.
  */
bool ppt_pop_rx_fifo_with_wait_option(uint8_t entry, uint8_t *pdu, uint16_t pdu_len, bool wait);

/**
  * @brief Wait for pop the received pdu from the rx fifo operation to be done.
  *
  * @return Operation result.
  * @retval True: Success.
  * @retval False: DMA error.
  */
bool ppt_pop_rx_fifo_wait_done(void);

/**
  * @brief Flush the rx fifo.
  *
  * Since the receiving state may be disrupted, some residual data needs to be flushed from the rx fifo.
  */
void ppt_flush_rx_fifo(void);

/**
  * @brief Flush the pending tx & rx interrupt.
  *
  * The tx & rx interrupt isn't cared about, so flush them.
  */
void ppt_flush_trx_int(void);

/**
  * @brief Check if it is receiving pkt.
  *
  * When the radio is in rx state, it may be waiting for a pkt or receiving a pkt.
  *
  * @return Whether receiving pkt.
  * @retval True: Is receiving pkt.
  * @retval False: Isn't receiving pkt.
  */
bool ppt_is_receiving_pkt(void);

/**
  * @brief Check if the radio is busy at running a command.
  *
  * When the radio is busy, it can't accept any new command,
  * and can't enter the dlps state.
  *
  * @return Whether busy.
  * @retval True: Is busy.
  * @retval False: Is idle.
  */
bool ppt_is_busy(void);

/**
  * @brief Get RSSI dbm value.
  *
  * @param[in] rssi_raw: RSSI register value, refer to @ref PRO_RSSI_TYPE.
  *
  * @return RSSI value.
  */
int8_t ppt_get_rssi(uint16_t rssi_raw);

/**
  * @brief Get channel setting from radio frequency.
  *
  * @param[in] frequency: Radio frequency in MHz.
  * @param[out] bank: Frequency band index.
  * @param[out] channel: Frequency channel offset.
  *
  * @return Result.
  * @retval True: The input radio frequency is supported by the module.
  * @retval False: The input radio frequency isn't supported by the module.
  */
bool ppt_get_bank_channel(uint16_t frequency, uint8_t *bank, uint8_t *channel);

/**
  * @brief Get BLE logic channel.
  *
  * @param[in] bank: Frequency band index.
  * @param[in] channel: Frequency channel offset.
  *
  * @return Logic channel index.
  * @retval >=0: The input channel is supported by BLE.
  * @retval <0: The input channel isn't supported by BLE.
  */
int8_t ppt_get_ble_logic_chan(uint8_t bank, uint8_t channel);

/**
  * @brief Get BLE logic channel.
  *
  * @param[in] frequency: Frequency in MHz.
  *
  * @return Logic channel index.
  * @retval >=0: The input channel is supported by BLE.
  * @retval <0: The input channel isn't supported by BLE.
  */
int8_t ppt_get_ble_logic_chan_via_freq(uint16_t frequency);

/**
  * @brief Get whitening init value from BLE logic channel.
  *
  * @param[in] le_logic_chan: BLE logic channel.
  *
  * @return Whitening init value.
  */
uint32_t ppt_get_ble_white_init(uint8_t le_logic_chan);

/**
  * @brief Format diagnose output with raw data.
  *
  * @param[in] data: Pointer of raw data.
  * @param[in] data_len: Length of raw data.
  * @param[in] fmt: Format string.
  */
void ppt_dump_log(uint8_t *data, uint32_t data_len, char *fmt, ...);

/**
  * @brief Dump all non-zero register.
  */
void ppt_dump_nonzero_reg(void);

/**
  * @brief Calculate the CRC of the designated data.
  *
  * The CRC engine is an easy to use CRC calculator.
  *
  * @param[in] param: CRC parameters.
  * @param[in] data: Pointer of raw data.
  * @param[in] data_len: Length of raw data in bytes for byte mode and in bits for bit mode, refer to bit_mode parameter in @ppt_crc_eng_param_t.
  *
  * @return CRC result.
  */
uint32_t ppt_crc_eng_cal(ppt_crc_eng_param_t *param, uint8_t *data, uint32_t data_len);

/**
  * @brief Initialize the dlps setting.
  *
  * This api shall be called early before entering dlps, for example in the pwr_mgr_init.
  */
void ppt_dlps_init(void);

/**
  * @brief Deinitialize the dlps setting when disabling 2.4g module.
  */
void ppt_dlps_deinit(void);

/**
  * @brief Register function that checks whether the 2.4g module can enter sleep mode.
  *
  * @param[in] check_cb: Function pointer of the dlps enter check callback.
  */
void ppt_dlps_reg_mac_check_cb(bool (*check_cb)(uint32_t *wakeup_time_diff));

/**
  * @brief Register function that stores some settings when the 2.4g module enters dlps.
  *
  * @param[in] enter_cb: Function pointer of the dlps enter store callback.
  */
void ppt_dlps_reg_mac_enter_cb(void (*enter_cb)(void));

/**
  * @brief Register function that restores some settings when the 2.4g module exits dlps.
  *
  * @param[in] exit_cb: Function pointer of the dlps exit restore callback.
  */
void ppt_dlps_reg_mac_exit_cb(void (*exit_cb)(void));

/**
  * @brief Register function that stores some settings when the platform enters dlps.
  *
  * @param[in] enter_cb: Function pointer of the dlps enter store callback.
  */
void ppt_dlps_reg_pf_enter_cb(void (*enter_cb)(void));

/**
  * @brief Register function that restores some settings when the platform exits dlps.
  *
  * @param[in] exit_cb: Function pointer of the dlps exit restore callback.
  */
void ppt_dlps_reg_pf_exit_cb(void (*exit_cb)(void));

/**
  * @brief Read the native time from the MAC hardware.
  *
  * The timer starts when mac is initialized, and is incremental.
  * Use @ref ppt_native_time_elapse to calculate elapsed time of two timestamps.
  *
  * @return Time in microseconds.
  */
uint64_t ppt_native_time_read(void);

/** @} End of PPT_Driver_Exported_Functions */

///@cond
#define TIME_DEBUG                  0

#if TIME_DEBUG
#include "rtl_rcc.h"
#include "rtl_pinmux.h"
#include "rtl_gpio.h"

#define TEST_PIN                    P2_2, P2_3, P2_4, P2_5
#define TIME_DEBUG_CHAN_INVALID     -1
#define TIME_DEBUG_CHAN_THREAD      0
#define TIME_DEBUG_CHAN_ISR         1
#define TIME_DEBUG_CHAN_ISR_TX      1
#define TIME_DEBUG_CHAN_ISR_RX      1
#define TIME_DEBUG_CHAN_ISR_GPIO    2
#define TIME_DEBUG_CHAN_TIMING      3
#define TIME_DEBUG_CHAN_DLPS        -1

extern uint8_t test_pin[];
extern uint32_t gpio_test_pin[];
extern bool level_test_pin[];
#define TIME_DEBUG_CHAN_NUMBER      COUNT_ARG(TEST_PIN)
void ppt_init_time_debug(void);

#define time_debug_level_high(ch)   do \
    { \
        if(ch >= 0 && ch < TIME_DEBUG_CHAN_NUMBER) \
        {\
            GPIOA->GPIO_DR |= gpio_test_pin[ch]; \
            level_test_pin[ch] = TRUE; \
        }\
    }while(0)
#define time_debug_level_low(ch)    do \
    { \
        if(ch >= 0 && ch < TIME_DEBUG_CHAN_NUMBER) \
        {\
            GPIOA->GPIO_DR &= ~gpio_test_pin[ch]; \
            level_test_pin[ch] = FALSE; \
        }\
    }while(0)
#define time_debug_edge_up(ch)      do \
    { \
        if(ch >= 0 && ch < TIME_DEBUG_CHAN_NUMBER) \
        {\
            GPIOA->GPIO_DR &= ~gpio_test_pin[ch]; \
            GPIOA->GPIO_DR |= gpio_test_pin[ch]; \
            level_test_pin[ch] = TRUE; \
        }\
    }while(0)
#define time_debug_edge_down(ch)    do \
    { \
        if(ch >= 0 && ch < TIME_DEBUG_CHAN_NUMBER) \
        {\
            GPIOA->GPIO_DR |= gpio_test_pin[ch]; \
            GPIOA->GPIO_DR &= ~gpio_test_pin[ch]; \
            level_test_pin[ch] = FALSE; \
        }\
    }while(0)
#else
#define time_debug_level_high(...)
#define time_debug_level_low(...)
#define time_debug_edge_up(...)
#define time_debug_edge_down(...)
#endif
void ppt_dbg_port_open(uint16_t bt_port, uint16_t ppt_port, uint16_t ppt_port2);
void ppt_dbg_port_set_pin(uint8_t pin_group, uint32_t dbg_bitmap);
void ppt_dbg_port(uint16_t bt_port, uint16_t ppt_port, uint16_t ppt_port2, uint32_t dbg_bitmap);
///@endcond

/** @} End of PPT_Driver */

#ifdef  __cplusplus
}
#endif      /*  __cplusplus */

#endif
