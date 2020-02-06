#include <lcom/lcf.h>

#include "kbc.h"
#include "mouse.h"
#include "utils.h"
#include "i8042.h"

#define PACKET_LEN 3

#define PACKET_LB_BIT 0
#define PACKET_RB_BIT 1
#define PACKET_MB_BIT 2
#define PACKET_CHECK_BIT 3
#define PACKET_SDX_BIT 4
#define PACKET_SDY_BIT 5
#define PACKET_XOVF_BIT 6
#define PACKET_YOVF_BIT 7

int mouse_hook_id = 23;

uint8_t packet_bytes[PACKET_LEN];
uint8_t packet_cnt = 0;

int mouse_subscribe_int(uint8_t *bit_no) {

	if(!bit_no)
		return INVAL_PTR;

	if(mouse_hook_id < 0 || mouse_hook_id > 31)
		return INVAL_BIT;

  // Send hook_id to caller
  *bit_no = mouse_hook_id;

	/*
		Subscribe to mouse interrupts
		IRQ_EXCLUSIVE needed to prevent minix KBC IH from getting notified
	*/
  int err = sys_irqsetpolicy(MOUSE_IRQ, IRQ_REENABLE | IRQ_EXCLUSIVE, &mouse_hook_id);

  sys_irq_print_error(err, "kbc_subscribe_int");

  return err;
}

int mouse_unsubscribe_int() {

  // Disable interrupts on the IRQ line associated with the specified hook_id
  int err = sys_irqdisable(&mouse_hook_id);
  sys_irq_print_error(err, "sys_irqdisable");

  if(err != OK)
    return err;

  // Unsubscribe interrupts on the IRQ line associated with the specified hook_id
  err = sys_irqrmpolicy(&mouse_hook_id);
  sys_irq_print_error(err, "sys_irqrmpolicy");

  return err;
}

void (mouse_ih)() {

  uint8_t temp;
  int err = OK;

  // Check status for errors
  if( (err = kbc_get_stat(&temp)) != OK)
    return;

	// Check for errors (bits 6 and 7)
	if( (temp & (KBC_PARITY | KBC_TIMEOUT)) != 0 ) {
		printf("Errors in KBC status: %x\n", temp);
		return;
	}

	// Read byte from output buffer --> ONLY READ 1 BYTE PER INTERRUPT
	err = util_sys_inb(KBC_OUT_BUF, &temp);

	if(err != OK) {
		printf("Failed to read byte from (output buffer) %d\n", KBC_OUT_BUF);
		return;
	}

	packet_bytes[packet_cnt++] = temp;
}

void mouse_ph() {		

  int err = OK;		
    	
  uint8_t data;		
    
  // Check status for errors		
  if( (err = kbc_get_stat(&data)) != OK)		
    return;		
    
  // loop until 8042 output buffer is full		
  if( (data & KBC_OBF) == 1 ) {		
    util_sys_inb(0x60, &data);		
  }		
    
  packet_bytes[packet_cnt++] = data;		
}

int get_mouse_packet(mouse_packet_t* mouse_packet) {

  if(packet_cnt == 1 && (packet_bytes[0] & BIT(PACKET_CHECK_BIT)) == 0)
  {
    packet_cnt = 0;
    return BAD_PACKET;
  }

  if(packet_cnt < PACKET_LEN)
    return BAD_PACKET;

  mouse_packet->lb = packet_bytes[0] & BIT(PACKET_LB_BIT);
  mouse_packet->rb = packet_bytes[0] & BIT(PACKET_RB_BIT);
  mouse_packet->mb = packet_bytes[0] & BIT(PACKET_MB_BIT);
  mouse_packet->x_ov = packet_bytes[0] & BIT(PACKET_XOVF_BIT);
  mouse_packet->y_ov = packet_bytes[0] & BIT(PACKET_YOVF_BIT);

  mouse_packet->delta_x = packet_bytes[1] + (((packet_bytes[0] & BIT(PACKET_SDX_BIT)) == 0) ? 0x0 : 0xFF00);
  mouse_packet->delta_y = packet_bytes[2] + (((packet_bytes[0] & BIT(PACKET_SDY_BIT)) == 0) ? 0x0 : 0xFF00);

  memcpy(mouse_packet->bytes, packet_bytes, PACKET_LEN);

  packet_cnt = 0;

  return OK;
}

int send_mouse_cmd(uint8_t* reply, uint8_t arg) {
  
  int err = OK;
  int retries = KBC_RETRIES;

  if( (err = kbc_write_cmd_w_arg(0xD4, arg)) != OK)
    return err;

  while( (retries--) > 0  ) {

    if( (err = util_sys_inb(KBC_OUT_BUF, reply)) != OK)
      continue;

    if(*reply == MS_NACK || *reply == MS_ERROR)
      continue;

    if(*reply == MS_ACK)
      return OK;
  }
  
  return MS_SEND_CMD_FAIL;
}

int read_gesture(mouse_packet_t* mouse_packet) {
  /*
  * STATE MACHINE
  * READ LB 1
  * DIAGONAL RIGHT/UP
  * READ LB 0
  * if (!READ RB 1) break;
  * READ RB 1
  * DIAGONAL LEFT/DOWN
  * READ RB 0
  */

  return OK;
}

uint8_t check_mouse_btn_state(const mouse_packet_t* packet) {
	uint8_t b = packet->bytes[0] & 0x07;
	return (b > 2) ? -1 : b; //TODO: new error code
}
