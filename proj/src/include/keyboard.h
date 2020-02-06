#ifndef __KEYBOARD_H
#define __KEYBOARD_H

#include <stdint.h>

#define SCANCODE_2_BYTE 0xE0

#define MAKECODE_W 0x11
#define MAKECODE_A 0x1e
#define MAKECODE_S 0x1f
#define MAKECODE_D 0x20
#define MAKECODE_P 0x19
#define MAKECODE_SPACE 0x39
#define MAKECODE_ESC 0x01
#define MAKECODE_1 0x02
#define MAKECODE_2 0x03
#define MAKECODE_3 0x04

#define BREAKCODE_W 0x91
#define BREAKCODE_A 0x9e
#define BREAKCODE_S 0x9f
#define BREAKCODE_D 0xa0
#define BREAKCODE_P 0x99
#define BREAKCODE_SPACE 0xb9
#define BREAKCODE_ESC 0x81
#define BREAKCODE_1 0x82
#define BREAKCODE_2 0x83
#define BREAKCODE_3 0x84

typedef struct scancode {
  bool make;
  uint8_t size;
  uint8_t bytes[2];
} Scancode_t;

/**
 * @brief Subscribe to interrupts from keyboard
 * 
 * @param bit_no bit used in notifications bitmask
 * @return int 0 if success, error code otherwise
 */
int kb_subscribe_int(uint8_t *bit_no);

/**
 * @brief Unsubscribe from keyboard interrupts
 * 
 * @return int 0 if success, error code otherwise
 */
int kb_unsubscribe_int();

/**
 * @brief Polling handler
 * 
 */
void kbc_ph();

/**
 * @brief Check if any bytes were read and try to processe them into a scancode.
 * 
 * @param key_code Pointer to be filled with new key code if scancode available
 * @param key_make Pointer to be filled with new key make/break if scancode available
 * @return int 0 if success, error code otherwise
 */
int get_scancode(uint8_t* key_code, bool* key_make);

#endif /* __KEYBOARD_H */
