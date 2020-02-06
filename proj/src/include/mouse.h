#ifndef __MOUSE_H__
#define __MOUSE_H__

#define BAD_PACKET 1

/**
 * @brief Information read from mouse interrupts
 * 
 */
typedef struct mouse_packet {
  uint8_t bytes[3]; /**< mouse packet raw bytes */
  bool rb, mb, lb;  /**< right, middle and left mouse buttons pressed */
  int16_t delta_x;  /**< mouse x-displacement: rightwards is positive */
  int16_t delta_y;  /**< mouse y-displacement: upwards is positive */
  bool x_ov, y_ov;  /**< mouse x-displacement and y-displacement overflows */
} mouse_packet_t;


/**
 * @brief Subscribe to interrupts from mouse
 * 
 * @param bit_no bit used in notifications bitmask
 * @return int 0 if success, error code otherwise
 */
int mouse_subscribe_int(uint8_t *bit_no);


/**
 * @brief Unsubscribe from mouse interrupts
 * 
 * @return int 0 if success, error code otherwise
 */
int mouse_unsubscribe_int();

/**
 * @brief Polling handler
 * 
 */
void mouse_ph();

/**
 * @brief Check if any bytes were read and try to processe them into a mouse packet.
 * 
 * @param packet Pointer to be filled with new mouse packet if available
 * @return int 0 if success, error code otherwise
 */
int mouse_get_packet(mouse_packet_t* packet);

/**
 * @brief Try to send mouse command to KBC
 * and wait for a reply
 * 
 * @param reply Pointer to be filled with KBC reply if command succeded
 * @param arg Command for mouse KBC
 * @return int 0 if success, error code otherwise
 */
int mouse_send_cmd(uint8_t* reply, uint8_t arg);


#endif /* __MOUSE_H__ */
