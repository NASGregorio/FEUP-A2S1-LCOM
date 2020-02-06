#ifndef __KEYBOARD_H
#define __KEYBOARD_H

#include <stdint.h>

#define SCANCODE_2_BYTE 0xE0
#define BREAKCODE_ESC 0x81

typedef struct scancode {
  bool make;
  uint8_t size;
  uint8_t bytes[2];
} Scancode_t;

int kb_subscribe_int(uint8_t *bit_no);

int kb_unsubscribe_int();

void kbc_ph();

int get_scancode(Scancode_t* code);

#endif /* __KEYBOARD_H */
