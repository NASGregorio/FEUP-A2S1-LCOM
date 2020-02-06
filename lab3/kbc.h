#ifndef __KBC_H__
#define __KBC_H__

#include <stdint.h>

int kbc_get_stat(uint8_t* stat);

int kbc_write_cmd(uint8_t cmd);

int kbc_write_cmd_w_arg(uint8_t cmd, uint8_t arg);

int kbc_read_from_reg(uint8_t reg, uint8_t* data);


#endif /* __KBC_H__ */
