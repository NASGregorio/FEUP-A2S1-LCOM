#include <lcom/lcf.h>

#include <stdint.h>
#include <errno.h>

#ifdef LAB3
int sys_inb_counter = 0;
#endif

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

  #ifdef LAB3
  sys_inb_counter++;
  #endif

  u32_t value32;
  int err = sys_inb(port, &value32);
  if(err != OK) {
    return err;
  }

  *value = (uint8_t)value32;

  return OK;
}

void sys_irq_print_error(int err, char* func_name) {
  switch (err)
  {
  case OK:
    break;
  case EINVAL:
    fprintf(stderr, "%s: Invalid request, IRQ line, hook id, or process number\n", func_name);
    break;
  case EPERM:
    fprintf(stderr, "%s: Only owner of hook can toggle interrupts or release the hook\n", func_name);
    break;
  case ENOSPC:
    fprintf(stderr, "%s: No free IRQ hook could be found\n", func_name);
    break;
  default:
    fprintf(stderr, "%s: Unspecified error: %u\n", func_name, err);
    break;
  }
}
