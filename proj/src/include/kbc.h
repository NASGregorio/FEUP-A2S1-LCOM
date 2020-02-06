#ifndef __KBC_H__
#define __KBC_H__

#include <stdint.h>

/**
 * @brief Get current KBC byte status
 * 
 * @param stat Pointer to be filled with KBC byte
 * @return int 0 if success, error code otherwise
 */
int kbc_get_stat(uint8_t* stat);

/**
 * @brief Try to send command cmd with argument arg to KBC 
 * 
 * @param cmd Command byte
 * @param arg Argument for command byte
 * @return int 0 if success, error code otherwise
 */
int kbc_write_cmd_w_arg(uint8_t cmd, uint8_t arg);

/**
 * @brief Try to write byte into register reg
 * Retries KBC_RETRIES times and stops
 * 
 * @param reg Register address
 * @param byte value to write
 * @return int 0 if success, error code otherwise
 */
int kbc_write_to_reg(uint8_t reg, uint8_t byte);

/**
 * @brief Try to read byte from register reg
 * Retries KBC_RETRIES times and stops
 * 
 * @param reg Register address
 * @param data Pointer to be filled with register value
 * @return int 0 if success, error code otherwise
 */
int kbc_read_from_reg(uint8_t reg, uint8_t* data);

#endif /* __KBC_H__ */
