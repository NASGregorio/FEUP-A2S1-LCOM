#include <lcom/lcf.h>

#include "mouse_manager.h"
#include "mouse.h"

mouse_state_t mouse_state;
mouse_packet_t mouse_packet;
const vec2du_t* screen_resolution;


int mouse_init(uint8_t* out_bit) {

  memset(&mouse_state, 0, sizeof(mouse_state_t));

  // Subscribe to keyboard interruptions
  if(mouse_subscribe_int(out_bit) != 0) {
    fprintf(stderr, "Failed to subscribe to mouse interrupt notifications\n");
    return 1;
  }

  return OK;

}

void mouse_destroy() {

  int err = mouse_unsubscribe_int();

  if(err != OK)
    printf("Bad mouse unsubscribe\n");
}

void mouse_set_boundaries(const vec2du_t* vg_resolution) {
  screen_resolution = vg_resolution;
}

int mouse_is_button_held(uint8_t key) {
  return ((mouse_state.buttons_hold >> key) & 0x1);
}

int mouse_is_button_down(uint8_t key) {
  return ((mouse_state.buttons_down >> key) & 0x1);
}

int mouse_is_button_up(uint8_t key) {
  return ((mouse_state.buttons_up >> key) & 0x1);
}

const vec2di_t* mouse_get_position() {
  return &(mouse_state.position);
}

void mouse_set_button_held(uint8_t key) {
  mouse_state.buttons_hold |= BIT(key);
}

void mouse_set_button_down(uint8_t key) {
  mouse_state.buttons_hold |= BIT(key);
  mouse_state.buttons_down |= BIT(key);
}

void mouse_set_button_up(uint8_t key) {
  mouse_state.buttons_hold &= ~(BIT(key));
  mouse_state.buttons_up |= BIT(key);
}

void mouse_clear_buttons_down_up() {
  mouse_state.buttons_down = 0;
  mouse_state.buttons_up = 0;
}

bool mouse_is_inside_area(box_t* area) {
  return  mouse_state.position.x >= area->position.x && mouse_state.position.x <= area->position.x + area->width &&
          mouse_state.position.y >= area->position.y && mouse_state.position.y <= area->position.y + area->height;
}


int mouse_update_state() {

  mouse_ih();

  if(mouse_get_packet(&mouse_packet) != OK)
    return -1;

  if(mouse_packet.lb) {
    if(!mouse_is_button_held(MS_LB)) { 
      mouse_set_button_down(MS_LB); 
    }
    mouse_set_button_held(MS_LB);
  }
  else if(mouse_is_button_held(MS_LB)) {
    mouse_set_button_up(MS_LB);
  }

  if(mouse_packet.rb) {
    if(!mouse_is_button_held(MS_RB)) { 
      mouse_set_button_down(MS_RB); 
    }
    mouse_set_button_held(MS_RB);
  }
  else if(mouse_is_button_held(MS_RB)) {
    mouse_set_button_up(MS_RB);
  }

  if(mouse_packet.mb) {
    if(!mouse_is_button_held(MS_MB)) { 
      mouse_set_button_down(MS_MB); 
    }
    mouse_set_button_held(MS_MB);
  }
  else if(mouse_is_button_held(MS_MB)) {
    mouse_set_button_up(MS_MB);
  }

  mouse_state.position.x += mouse_packet.delta_x;
  mouse_state.position.y -= mouse_packet.delta_y;

  mouse_state.position.x = MIN(screen_resolution->x-1, mouse_state.position.x);
  mouse_state.position.y = MIN(screen_resolution->y-1, mouse_state.position.y);
  mouse_state.position.x = MAX(0, mouse_state.position.x);
  mouse_state.position.y = MAX(0, mouse_state.position.y);

	return OK;
}

void print_mouse_state() {
  printf("hold %u | ", mouse_state.buttons_hold);
  printf("down %u | ", mouse_state.buttons_down);
  printf("up   %u", mouse_state.buttons_up);
  printf(" @ %d:%d\n", mouse_state.position.x, mouse_state.position.y);
}
