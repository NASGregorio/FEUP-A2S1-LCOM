#include <lcom/lcf.h>

#include "kbc.h"
#include "i8042.h"

int kbc_read_from_reg(uint8_t reg, uint8_t* data) {
  int err = OK;
  uint8_t temp;
  int retries = KBC_RETRIES;

  while( (retries--) > 0 ) {

    // Check status for errors
    if( (err = kbc_get_stat(&temp)) != OK)
      return INVAL_STAT;

    //loop until 8042 reg is full
    if( (temp & KBC_OBF) == 1 ) {

      err = util_sys_inb(reg, data);

      if(err != OK) {
        printf("Failed to read data from %d\n", reg);
        return err;
      }

      return ( (*data &(KBC_PARITY | KBC_TIMEOUT)) == 0 ) ? OK : KBC_OUTBUF_ERR;
    }
    tickdelay(micros_to_ticks(KBC_DELAY));
  }
  return KBC_RETRY_ERR;
}

int kbc_write_to_reg(uint8_t reg, uint8_t byte) {
  int err = OK;
  uint8_t temp;
  uint8_t retries = KBC_RETRIES;

  while( (retries--) > 0 ) {

    // Check status for errors
    if( (err = kbc_get_stat(&temp)) != OK)
      return INVAL_STAT;

    // loop until 8042 input buffer is empty
    if( (temp & KBC_IBF) == 0 )
      return sys_outb(reg, byte);

    // wait for KBC_DELAY microseconds
    tickdelay(micros_to_ticks(KBC_DELAY));
  }
  return KBC_RETRY_ERR;
}

int kbc_get_stat(uint8_t* stat) {
  
    int err = util_sys_inb(KBC_STAT_REG, stat);
    if(err != OK) {
      printf("Failed to read from (status register) %d\n", KBC_STAT_REG);
      return INVAL_STAT;
    }
    return OK;
}

int kbc_write_cmd_w_arg(uint8_t cmd, uint8_t arg) {
  int err = OK;
  
  if( (err = kbc_write_to_reg(KBC_CMD_REG, cmd)) != OK)
    return err;

  uint8_t temp = 0;
  // Check status for errors
  if( (err = kbc_get_stat(&temp)) != OK)
    return INVAL_STAT;

  return sys_outb(KBC_IN_BUF, arg);

  //return kbc_write_to_reg(KBC_IN_BUF, arg);
}
