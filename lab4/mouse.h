#ifndef __MOUSE_H__
#define __MOUSE_H__

#define BAD_PACKET 1

#define MS_LBTN 1
#define MS_RBTN 2
#define MS_MBTN 3

typedef struct mouse_packet {
  uint8_t bytes[3]; // mouse packet raw bytes
  bool rb, mb, lb;  // right, middle and left mouse buttons pressed
  int16_t delta_x;  // mouse x-displacement: rightwards is positive
  int16_t delta_y;  // mouse y-displacement: upwards is positive
  bool x_ov, y_ov;  // mouse x-displacement and y-displacement overflows
} mouse_packet_t;

int mouse_subscribe_int(uint8_t *bit_no);

int mouse_unsubscribe_int();

int get_mouse_packet(mouse_packet_t* mouse_packet);

int send_mouse_cmd(uint8_t* reply, uint8_t arg);

void mouse_ph();

uint8_t check_mouse_btn_state(const mouse_packet_t* packet);

#endif /* __MOUSE_H__ */
