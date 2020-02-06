#include <lcom/lcf.h>

#include "kb_manager.h"
#include "keyboard.h"

kb_state_t kb_state;
uint8_t key_code;
bool key_make;

int kb_init(uint8_t* out_bit) {

  memset(&kb_state, 0, sizeof(kb_state_t));

  if(kb_subscribe_int(out_bit) != 0) {
    fprintf(stderr, "Failed to subscribe to timer interrupt notifications\n");
    return 1;
  }

  return OK;
}

void kb_destroy() {

  int err = kb_unsubscribe_int();

  if(err != OK)
    printf("Bad kb unsubscribe\n");
}


void kb_set_key_down(uint8_t key) {
  kb_state.keys_hold |= BIT(key);
  kb_state.keys_down |= BIT(key);
}

void kb_set_key_up(uint8_t key) {
  kb_state.keys_hold &= ~(BIT(key));
  kb_state.keys_up |= BIT(key);
}

void kb_clear_keys_down_up() {
  kb_state.keys_down = 0;
  kb_state.keys_up = 0;
}


int kb_is_key_held(uint8_t key) {
  return ((kb_state.keys_hold >> key) & 0x1);
}
int kb_is_key_down(uint8_t key) {
  return ((kb_state.keys_down >> key) & 0x1);
}
int kb_is_key_up(uint8_t key) {
  return ((kb_state.keys_up >> key) & 0x1);
}



void kb_update_key(uint8_t key) {
    if(key_make)
        kb_set_key_down(key);
    else
        kb_set_key_up(key);
}

int kb_update_keys() {

  kbc_ih();

  if(get_scancode(&key_code, &key_make) != OK)
    return -1;

  switch (key_code)
  {
  case MAKECODE_W:
    kb_update_key( KEY_W);
    break;
  case MAKECODE_A:
    kb_update_key( KEY_A);
    break;
  case MAKECODE_S:
    kb_update_key( KEY_S);
    break;
  case MAKECODE_D:
    kb_update_key( KEY_D);
    break;
  case MAKECODE_P:
    kb_update_key( KEY_P);
    break;
  case MAKECODE_1:
    kb_update_key( KEY_1);
    break;
  case MAKECODE_2:
    kb_update_key( KEY_2);
    break;
  case MAKECODE_3:
    kb_update_key( KEY_3);
    break;
  case MAKECODE_SPACE:
    kb_update_key( KEY_SPACE);
    break;
  case MAKECODE_ESC:
    kb_update_key( KEY_ESC);
    break;
  default:
      break;
  }

  return OK;
}

void print_kb_state() {
  printf("hold %u | ", kb_state.keys_hold);
  printf("down %u | ", kb_state.keys_down);
  printf("up   %u\n", kb_state.keys_up);
}
