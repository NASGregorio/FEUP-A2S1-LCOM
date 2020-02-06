#ifndef __MOUSE_MANAGER_H__
#define __MOUSE_MANAGER_H__

#include <stdint.h>
#include "vector2d.h"
#include "box.h"

#define MS_LB 0
#define MS_RB 1
#define MS_MB 2

typedef uint8_t buttons_t;

/**
 * @brief Holds mouse position and bitmasks for button state tracking
 * 
 */
typedef struct {
  buttons_t buttons_hold;  /**< Bitmask for buttons being held */
  buttons_t buttons_down;  /**< Bitmask for buttons pressed in the current frame */
  buttons_t buttons_up;    /**< Bitmask for buttons released in the current frame */
  vec2di_t position;       /**< Current position of the mouse within the screen */
} mouse_state_t;

/**
 * @brief Initialize mouse manager and device
 * 
 * @param out_bit bit value to be filled on subscribe
 * @return int execution return code
 */
int mouse_init(uint8_t* out_bit);

/**
 * @brief Clean up keyboard manager and unsubscribe from device
 * 
 */
void mouse_destroy();

/**
 * @brief Sets mouse boundaries accordint o vg_resolution
 * 
 * @param vg_resolution Pointer to current video mode's screen resolution
 */
void mouse_set_boundaries(const vec2du_t* vg_resolution);

/**
 * @brief Check if button is being held
 * 
 * @param key bit corresponding to a mouse button
 * @return int 1 for held or 0 for not held
 */
int mouse_is_button_held(uint8_t key);

/**
 * @brief  Check if button is down in this frame
 * 
 * @param key bit corresponding to a button
 * @return int 1 for down or 0 for not down
 */
int mouse_is_button_down(uint8_t key);

/**
 * @brief  Check if button is up in this frame
 * 
 * @param key bit corresponding to a button
 * @return int 1 for up or 0 for not up
 */
int mouse_is_button_up(uint8_t key);

/**
 * @brief Get current mouse position
 * 
 * @return const vec2di_t* Pointer to mouse's position
 */
const vec2di_t* mouse_get_position();

/**
 * @brief Check if mouse is inside a certain area
 * 
 * @param area area to be checked
 * @return true if mouse is not inside
 * @return false if mouse is inside
 */
bool mouse_is_inside_area(box_t* area);

/**
 * @brief Read from KBC and update the mouse.
 * (Function to be called with every mouse interruption)
 * 
 * @return int 0 if a new scancode was processed. Otherwise -1.
 */
int mouse_update_state();

/**
 * @brief Set all bits in bitmasks to zero
 * 
 */
void mouse_clear_buttons_down_up();

/**
 * @brief Print mouse bitmasks
 * 
 */
void print_mouse_state();

#endif /* __MOUSE_MANAGER_H__ */
