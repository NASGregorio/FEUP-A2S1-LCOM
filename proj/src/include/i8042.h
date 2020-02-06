#ifndef __LCOM_I8042_H__
#define __LCOM_I8042_H__

#include <lcom/lcf.h>


/** @defgroup i8042 i8042
 * @{
 *
 * Constants for programming the i8042 controller.
 */

#define KB_IRQ 1
#define MOUSE_IRQ 12

#define KBC_STAT_REG 0x64
#define KBC_CMD_REG 0x64
#define KBC_IN_BUF 0x60
#define KBC_OUT_BUF 0x60

#define KBC_ASK_CMDB 0x20
#define KBC_WRT_CMDB 0x60

#define KBC_DELAY 20000
#define KBC_RETRIES 10

///////////////////////////////////////////

#define KBC_OBF_BIT 0
#define KBC_IBF_BIT 1
#define KBC_AUX_BIT 5
#define KBC_TIMEOUT_BIT 6
#define KBC_PARITY_BIT 7

#define KBC_OBF BIT(KBC_OBF_BIT)
#define KBC_IBF BIT(KBC_IBF_BIT)
#define KBC_AUX BIT(KBC_AUX_BIT)
#define KBC_TIMEOUT BIT(KBC_TIMEOUT_BIT)
#define KBC_PARITY BIT(KBC_PARITY_BIT)

///////////////////////////////////////////

#define KBC_INT_KB_BIT 0
#define KBC_INT_MS_BIT 1

#define KBC_INT_KB BIT(KBC_INT_KB_BIT)
#define KBC_INT_MS BIT(KBC_INT_MS_BIT)

///////////////////////////////////////////

#define MS_ENABLE_DATA_REPORT 0xF4
#define MS_DISABLE_DATA_REPORT 0xF5
#define MS_SET_REMOTE_MODE 0xF0
#define MS_SET_STREAM_MODE 0xEA
#define MS_READ_DATA 0xEB

#define MS_ACK 0xFA
#define MS_NACK 0xFE
#define MS_ERROR 0xFC

///////////////////////////////////////////

#define INVAL_PTR 1
#define INVAL_BIT 2
#define INVAL_STAT 3
#define KBC_RETRY_ERR 4
#define KBC_OUTBUF_ERR 5
#define MS_SEND_CMD_FAIL 6

///////////////////////////////////////////

#endif /* __LCOM_I8042_H__ */
