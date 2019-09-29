#include <lcom/lcf.h>
#include <lcom/timer.h>

#include <stdint.h>

#include "i8254.h"

int (timer_set_frequency)(uint8_t timer, uint32_t freq) {

  // Check if timer is within valid range
  if(timer < 0 || timer > 2) {
    fprintf(stderr, "Invalid timer selected: %d\n", timer);
    return 1;
  }

  uint8_t ctrlWord = 0;

  // escolher timer com bit 6 e 7
  switch (timer) //BIT 6 and 7
	{
	case 0:
	  ctrlWord &= ~(TIMER_SEL1 | TIMER_SEL2);
	  break;
	case 1:
	  ctrlWord |= TIMER_SEL1;
	  break;
	case 2:
	  ctrlWord |= TIMER_SEL2;
	  break;
	default:
	  break;
	}

  ctrlWord |= TIMER_LSB_MSB;


  // Read back e guardar bits 0, 1, 2, 3
  uint8_t st;
  if(timer_get_conf(timer, &st) != 0) {
    fprintf(stderr, "Failed to get timer config\n");
    return 1;
  }

  ctrlWord |= (st & 0x0F);


  // calcular initial counting value = X
  // TIMER_FREQ / X = freq
  // X = TIMER_FREQ / freq
  uint16_t init_count = TIMER_FREQ / freq;

  // dividir initial counting value em lsb e msb (bit 4 e 5 respectivamente)
  uint8_t lsb;
  util_get_LSB(init_count, &lsb);
  uint8_t msb;
  util_get_MSB(init_count, &msb);

  // Write control word command in control register
  if(sys_outb(TIMER_CTRL, ctrlWord) == EINVAL) {
    //fprintf(stderr, "Failed to write (RB) %d in (control register) %d\n", RB_CMD, TIMER_CTRL);
    return 1;
  }

  if(sys_outb(TIMER_0 + timer, lsb) == EINVAL) {
    //fprintf(stderr, "Failed to write (RB) %d in (control register) %d\n", RB_CMD, TIMER_CTRL);
    return 1;
  }

  if(sys_outb(TIMER_0 + timer, msb) == EINVAL) {
    //fprintf(stderr, "Failed to write (RB) %d in (control register) %d\n", RB_CMD, TIMER_CTRL);
    return 1;
  }

  return 0;
}

int (timer_subscribe_int)(uint8_t *bit_no) {
    /* To be implemented by the students */
  printf("%s is not yet implemented!\n", __func__);

  return 1;
}

int (timer_unsubscribe_int)() {
  /* To be implemented by the students */
  printf("%s is not yet implemented!\n", __func__);

  return 1;
}

void (timer_int_handler)() {
  /* To be implemented by the students */
  printf("%s is not yet implemented!\n", __func__);
}

int (timer_get_conf)(uint8_t timer, uint8_t *st) {

  // Check if timer is within valid range
  if(timer < 0 || timer > 2) {
    fprintf(stderr, "Invalid timer selected: %d\n", timer);
    return 1;
  }

  // Prepare read-back command (COUNT & STATUS -> active low)
  u32_t RB_CMD = 0;
  RB_CMD |= (TIMER_RB_CMD | TIMER_RB_COUNT_ | TIMER_RB_SEL(timer)); //ON
  RB_CMD &= ~(TIMER_RB_STATUS_ & BIT(0)); //OFF

  // Write read-back command in control register
  if(sys_outb(TIMER_CTRL, RB_CMD) == EINVAL) {
    fprintf(stderr, "Failed to write (RB) %d in (control register) %d\n", RB_CMD, TIMER_CTRL);
    return 1;
  }

  // Read from requested timer register
  int err = 0;
  switch (timer)
  {
  case 0:
    err = util_sys_inb(TIMER_0, st);
    break;
  case 1:
    err = util_sys_inb(TIMER_1, st);
    break;
  case 2:
    err = util_sys_inb(TIMER_2, st);
    break;
  default:
    err = EINVAL;
    break;
  }

  if(err == EINVAL) {
    fprintf(stderr, "Failed to read from (timer register) %d\n", timer);
    return 1;
  }

  return 0;
}

const char *bit_rep[16] = {
    [ 0] = "0000", [ 1] = "0001", [ 2] = "0010", [ 3] = "0011",
    [ 4] = "0100", [ 5] = "0101", [ 6] = "0110", [ 7] = "0111",
    [ 8] = "1000", [ 9] = "1001", [10] = "1010", [11] = "1011",
    [12] = "1100", [13] = "1101", [14] = "1110", [15] = "1111",
};

void print_byte(int d, __uint8_t byte)
{
    printf("%d: %s%s\n", d, bit_rep[byte >> 4], bit_rep[byte & 0x0F]);
}

int (timer_display_conf)(uint8_t timer, uint8_t st,
                        enum timer_status_field field) {

  // Check if timer is within valid range
  if(timer < 0 || timer > 2) {
    fprintf(stderr, "Invalid timer selected: %d\n", timer);
    return 1;
  }

  union timer_status_field_val conf;
  uint8_t val = 0;

  switch (field)
  {
  // Add whole status byte - BITs 0-7
  case tsf_all:
    conf.byte = st;
    break;

  // Add initialization mode - BITs 4 and 5
  case tsf_initial:
    val = (st & TIMER_LSB_MSB) >> TIMER_LSB_BIT;
    if(val < 1 || val > 3)
      return 1;
    conf.in_mode = (enum timer_init)val;
    break;

  // Add counting mode - BITs 1, 2 and 3
  case tsf_mode:
    val = (st & (BIT(3) | BIT(2) | BIT(1))) >> 1;
    val -= (val > 5) ? 4 : 0;
    conf.count_mode = val;
    break;

  // Add counting base - BIT 0
  case tsf_base:
    val = (st & TIMER_BCD);
    conf.bcd = val;
    break;

  default:
    fprintf(stderr, "Invalid timer field: %d\n", field);
    return 1;
    break;
  }

  // Print timer config
  if(timer_print_config(timer, field, conf) != 0) {
    fprintf(stderr, "Failed to print (field) %u from (timer) %d\n", field, timer);
    return 1;
  }

  return 0;
}
