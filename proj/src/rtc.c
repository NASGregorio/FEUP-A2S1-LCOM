#include <lcom/lcf.h>

#include "rtc.h"

#define RTC_IRQ 8
#define RTC_ADDR_REG 0x70
#define RTC_DATA_REG 0x71

#define RTC_A_UIP BIT(7)

#define RTC_B_UIE BIT(4)
#define RTC_B_SET BIT(7)
#define RTC_B_24H BIT(1)

#define RTC_REG_A 0x0A
#define RTC_REG_B 0x0B	
#define RTC_REG_C 0x0C
#define RTC_REG_D 0x0D

#define INVAL_PTR 1
#define INVAL_BIT 2
#define INVAL_READ 3
#define INVAL_PARAM 4

int rtc_hook_id = 24;
uint8_t rtc_reg_a = 0;
uint8_t rtc_reg_b = 0;


void rtc_disable_int() {
  sys_irq_print_error( sys_irqdisable(&rtc_hook_id), "sys_irqdisable" );
}

void rtc_enable_int() {
  sys_irq_print_error( sys_irqenable(&rtc_hook_id), "sys_irqenable" );
}

int rtc_subscribe_int(uint8_t *bit_no) {

	if(!bit_no)
		return INVAL_PTR;

	if(rtc_hook_id < 0 || rtc_hook_id > 31)
		return INVAL_BIT;

  // Send hook_id to caller
  *bit_no = rtc_hook_id;

  /*
      Subscribe to mouse interrupts
      IRQ_EXCLUSIVE needed to prevent minix KBC IH from getting notified
  */
  int err = sys_irqsetpolicy(RTC_IRQ, IRQ_REENABLE | IRQ_EXCLUSIVE, &rtc_hook_id);

  sys_irq_print_error(err, "rtc_subscribe_int");

  return err;
}

int rtc_unsubscribe_int() {

  // Disable interrupts on the IRQ line associated with the specified hook_id
  rtc_disable_int();

  // Unsubscribe interrupts on the IRQ line associated with the specified hook_id
  int err = sys_irqrmpolicy(&rtc_hook_id);
  sys_irq_print_error(err, "sys_irqrmpolicy");

  return err;
}

int rtc_read_from_reg(uint8_t reg, uint8_t* data) {

  if(sys_outb(RTC_ADDR_REG, reg) != OK) {
    return INVAL_READ;
  }

  if(util_sys_inb(RTC_DATA_REG, data) != OK) {
    return INVAL_READ;
  }

  return 0;
}

void wait_valid_rtc() {

  do {

    rtc_disable_int();
    rtc_read_from_reg(RTC_REG_A, &rtc_reg_a);
    rtc_enable_int();
    
  } while ( rtc_reg_a & RTC_A_UIP);

}

int hex_2_dec(uint8_t x){
    return (((x & 0xF0) >> 4) * 10) + (x & 0x0F);
}

int rtc_init(uint8_t* out_bit) {

  if(rtc_read_from_reg(RTC_REG_B, &rtc_reg_b) == -1) {
    return -1;
  }

  //Enable update interruptions
  rtc_reg_b |= RTC_B_UIE;

  //Allow updates of time/date registers
  rtc_reg_b &= ~RTC_B_SET;

  //Set hours range from 0 to 23
  rtc_reg_b |= RTC_B_24H;

  // if(sys_outb(RTC_ADDR_REG, RTC_REG_B) != OK) {
	// 	return -1;
  // }

	if(sys_outb(RTC_DATA_REG, rtc_reg_b) != OK) {
		return -1;
  }

  // Subscribe to keyboard interruptions
  if(rtc_subscribe_int(out_bit) != 0) {
    fprintf(stderr, "Failed to subscribe to RTC interrupt notifications\n");
    return 1;
  }

  return 0;
}

void rtc_destroy() {
  int err = rtc_unsubscribe_int();

  if(err != OK)
    printf("Bad rtc unsubscribe\n");
}

int rtc_get_param(uint8_t param, uint8_t* value) {

  if( param != RTC_REG_SEC && 
      param != RTC_REG_MIN && 
      param != RTC_REG_HOUR && 
      param != RTC_REG_WEEK &&
      param != RTC_REG_DMOT &&
      param != RTC_REG_MOT &&
      param != RTC_REG_YEAR) {
    return INVAL_PARAM;
  }

  wait_valid_rtc();

	if(rtc_read_from_reg(param, value) == -1) {
    return -1;
  }

  *value = hex_2_dec(*value);

	return OK;
}
