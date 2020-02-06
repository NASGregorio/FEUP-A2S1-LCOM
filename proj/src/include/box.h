#ifndef __BOX_H__
#define __BOX_H__

#include "vector2d.h"

/**
 * @brief AABB structure
 * 
 */
typedef struct box {
  vec2di_t old_position;  /**< Last frame position */
  vec2di_t position;      /**< Current frame position */
  vec2di_t velocity;      /**< Current frame velocity */
  uint16_t width;         /**< Box width */
  uint16_t height;        /**< Box height */
} box_t;

/**
 * @brief Check if box is out of screen
 * 
 * @param box box to check
 * @param vg_resolution screen resolution
 * @return true 
 * @return false 
 */
bool check_box_out_of_screen(box_t* box, const vec2du_t* vg_resolution);

/**
 * @brief Check if box b1 is colliding with box b2
 * 
 * @param b1 Box 1
 * @param b2 Box 2
 * @return true 
 * @return false 
 */
bool check_simple_collision(box_t* b1, box_t* b2);

/**
 * @brief Check if box is colliding with pushable box and
 * push pushable box if so.
 * 
 * @param box Box that pushes
 * @param pushable Box that is pushed
 */
void collision_resolution(const box_t* box, box_t* pushable);

#endif /* __BOX_H__ */

