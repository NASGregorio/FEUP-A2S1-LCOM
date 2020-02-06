#include <lcom/lcf.h>

#include <stdint.h>
#include <errno.h>

int(util_get_LSB)(uint16_t val, uint8_t *lsb) {

  if(!lsb)
    return 1;

  *lsb = (__uint8_t)val;

  return 0;
}

int(util_get_MSB)(uint16_t val, uint8_t *msb) {

  if(!msb)
    return 1;

	*msb = val >> 8;

  return 0;
}

int (util_sys_inb)(int port, uint8_t *value) {

  u32_t value32;
  if(sys_inb(port, &value32) == EINVAL) {
    return 1;
  }

  // // Read first byte from 4 byte type
  // uint8_t* b = (uint8_t*)&value32;
  // // Place it in provided address
  // *value = b[0];

  *value = ((__uint8_t*)&value32)[0];

  return 0;
}
