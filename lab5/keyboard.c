#include <lcom/lcf.h>

#include "kbc.h"
#include "keyboard.h"
#include "utils.h"
#include "i8042.h"

int kb_hook_id = 22;

uint8_t scancode_bytes[2];
uint8_t scancode_bytes_cnt = 0;

int kb_subscribe_int(uint8_t *bit_no) {

	if(!bit_no)
		return INVAL_PTR;

	if(kb_hook_id < 0 || kb_hook_id > 31)
		return INVAL_BIT;

	// Send hook_id to caller
	*bit_no = kb_hook_id;

	/*
		Subscribe to keyboard interrupts
		IRQ_EXCLUSIVE needed to prevent minix KBC IH from getting notified
	*/
	int err = sys_irqsetpolicy(KB_IRQ, IRQ_REENABLE | IRQ_EXCLUSIVE, &kb_hook_id);

	sys_irq_print_error(err, "kbc_subscribe_int");

	return err;
}

int kb_unsubscribe_int() {

	// Disable interrupts on the IRQ line associated with the specified hook_id
	int err = sys_irqdisable(&kb_hook_id);
	sys_irq_print_error(err, "sys_irqdisable");

	if(err != OK)
		return err;

	// Unsubscribe interrupts on the IRQ line associated with the specified hook_id
	err = sys_irqrmpolicy(&kb_hook_id);
	sys_irq_print_error(err, "sys_irqrmpolicy");

	return err;
}

void (kbc_ih)() {

	uint8_t temp;
	int err = OK;

  // Check status for errors
  if( (err = kbc_get_stat(&temp)) != OK)
    return;

	// Check for errors (bits 6 and 7)
	if( ((temp & (KBC_PARITY | KBC_TIMEOUT)) >> KBC_TIMEOUT_BIT) != 0 ) {
		printf("Errors in KBC status: %x\n", temp);
		return;
	}

	// Read byte from output buffer --> ONLY READ 1 BYTE PER INTERRUPT
	err = util_sys_inb(KBC_OUT_BUF, &temp);

	if(err != OK) {
		printf("Failed to read byte from (output buffer) %d\n", KBC_OUT_BUF);
		return;
	}

	scancode_bytes[scancode_bytes_cnt++] = temp;
}

void kbc_ph() {
  uint8_t temp;
  int err = OK;

	// Check status for errors
  if( (err = kbc_get_stat(&temp)) != OK)
    return;

  if( (temp & KBC_OBF) == 0  || (temp & KBC_AUX) != 0)
		return;

  // Check for errors (bits 6 and 7)
  if( ((temp & (KBC_PARITY | KBC_TIMEOUT)) >> KBC_TIMEOUT_BIT) != 0 ) {
		printf("Errors in KBC status: %x\n", temp);
		return;
  }

  // Read byte from output buffer --> ONLY READ 1 BYTE PER INTERRUPT
  err = util_sys_inb(KBC_OUT_BUF, &temp);

  if(err != OK) {
		printf("Failed to read byte from (output buffer) %d\n", KBC_OUT_BUF);
		return;
  }

  scancode_bytes[scancode_bytes_cnt++] = temp;
}

int get_scancode(Scancode_t* code) {

  if(!code)
	return -1;

  if(scancode_bytes_cnt == 0)
	return -1;

  // check if byte_code == 0xE0
  if(scancode_bytes_cnt == 1 && (scancode_bytes[0] == SCANCODE_2_BYTE) )
	return -1;

  code->make = ((scancode_bytes[scancode_bytes_cnt-1] & 0x80) == 0);
  code->size = scancode_bytes_cnt;
  memcpy(code->bytes, scancode_bytes, scancode_bytes_cnt);

  scancode_bytes_cnt = 0;

  return 0;
}
