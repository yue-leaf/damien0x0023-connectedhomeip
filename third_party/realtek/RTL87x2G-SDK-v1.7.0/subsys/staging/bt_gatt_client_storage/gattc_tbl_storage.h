#ifndef _GATTC_TBL_STORAGE_H_
#define _GATTC_TBL_STORAGE_H_

#ifdef  __cplusplus
extern "C" {
#endif      /* __cplusplus */

#include "bt_ext_ftl.h"
#include "gap_callback_le.h"

#if GATTC_TBL_STORAGE_SUPPORT

/** @defgroup GATTC_FTL_DEV_NUM
  * @brief Maximum device number that the GATT client save into FTL.
  *        This number shall be no less than the LE bond device number configured in the MCUConfigTool.
  * @{
  */
#define GATTC_FTL_MAX_DEV_NUM   8
/** @} End of GATTC_FTL_DEV_NUM */

#define BT_EXT_FTL_BLOCK_LEN     128
#define BT_EXT_FTL_PARTITION_NAME "BT_FTL"
#define BT_EXT_FTL_SIZE 0x2000

void gattc_tbl_storage_init(void);
void gattc_tbl_storage_handle_bond_modify(T_LE_BOND_MODIFY_INFO *p_info);
bool gattc_tbl_storage_del(uint16_t conn_handle);

#endif

#ifdef  __cplusplus
}
#endif      /*  __cplusplus */

#endif
