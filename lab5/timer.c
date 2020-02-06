#include <lcom/lcf.h>
#include <lcom/timer.h>

#include <stdint.h>

#include "i8254.h"
#include "utils.h"

int timer_hook_id = 21;   // Id for interrupt notification

int irq_counter=0; // Interger incremented by interrupt handler


int (timer_set_frequency)(uint8_t timer, uint32_t freq) {

  // Check if timer is within valid range
  if(timer < 0 || timer > 2) {
    fprintf(stderr, "Invalid timer selected: %d\n", timer);
    return 1;
  }

  // Check if frequency is higher than the minimum allowed
  if(freq < (TIMER_FREQ/(BIT(16)-1))+1) {
    fprintf(stderr, "Frequency must be above 18 Hz\n");
    return 1;
  }

  uint8_t ctrlWord = 0;

  // Select timer using bits 6 and 7
  switch (timer)
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

  // Set initialization mode using bits 4 and 5
  ctrlWord |= TIMER_LSB_MSB; 

  // Get current timer config 
  uint8_t st;
  if(timer_get_conf(timer, &st) != 0) {
    fprintf(stderr, "Failed to get timer config\n");
    return 1;
  }

  // Set least significant byte from current timer config
  ctrlWord |= (st & 0x0F);

  /*
  * Calculate initial counting value = init_count
  * TIMER_FREQ / init_count = freq
  * init_count = TIMER_FREQ / freq
  */
  uint16_t init_count = TIMER_FREQ / freq;

  // Split initial counting value into lsb and msb (bits 4 and 5 respectively)
  uint8_t lsb;
  util_get_LSB(init_count, &lsb);
  uint8_t msb;
  util_get_MSB(init_count, &msb);
  

  // Write control word command in control register
  if(sys_outb(TIMER_CTRL, ctrlWord) == EINVAL) {
    fprintf(stderr, "Failed to write (WC) %d in (control register) %d\n", ctrlWord, TIMER_CTRL);
    return 1;
  }

  int timer_port;
  switch (timer)
  {
  case 0:
    timer_port = TIMER_0;
    break;
  case 1:
    timer_port = TIMER_1;
    break;
  case 2:
    timer_port = TIMER_2;
    break;
  default:
    timer_port = -1;
    break;
  }

  // Write frequency's LSB followed by MSB into timer register
  if(sys_outb(timer_port, lsb) == EINVAL) {
    fprintf(stderr, "Failed to write (LSB) %d in (timer register) %d\n", lsb, timer_port);
    return 1;
  }

  if(sys_outb(timer_port, msb) == EINVAL) {
    fprintf(stderr, "Failed to write (MSB) %d in (timer register) %d\n", msb, timer_port);
    return 1;
  }

  return 0;
}

int (timer_subscribe_int)(uint8_t *bit_no) {

  if(!bit_no)
    return 1;

  // Send hook_id to caller
  *bit_no = timer_hook_id;

  /* 
  * Subscribe to notification from interrupt on TIMER_0
  * Setting policy as IRQ_REENABLE automatically enables interrupts 
  * therefore sys_irqenable doesn't have to be called explicitly
  */
  int err = sys_irqsetpolicy(TIMER0_IRQ, IRQ_REENABLE, &timer_hook_id);

  sys_irq_print_error(err, "sys_irqsetpolicy");

  return err;
}

int (timer_unsubscribe_int)() {

  // Disable interrupts on the IRQ line associated with the specified hook_id
  int err = sys_irqdisable(&timer_hook_id);

  sys_irq_print_error(err, "sys_irqdisable");

  if(err != OK)
    return err;

  // Unsubscribe interrupts on the IRQ line associated with the specified hook_id
  err = sys_irqrmpolicy(&timer_hook_id);

  sys_irq_print_error(err, "sys_irqrmpolicy");

  return err;
}

void (timer_int_handler)() {
  irq_counter++;
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
    conf.in_mode = (val < 1 || val > 3) ? INVAL_val : (enum timer_init)val;
    break;

  // Add counting mode - BITs 1, 2 and 3
  case tsf_mode:
    val = (st & (BIT(3) | BIT(2) | BIT(1))) >> 1;
    val -= (val > 5) ? 4 : 0; // Set counting modes 6 and 7 to 2 and 3
    conf.count_mode = val;
    break;

  // Add counting base - BIT 0
  case tsf_base:
    conf.bcd = (st & TIMER_BCD);
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
