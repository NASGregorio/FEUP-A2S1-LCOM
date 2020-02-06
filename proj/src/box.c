#include <lcom/lcf.h>

#include "box.h"

bool check_box_out_of_screen(box_t* box, const vec2du_t* vg_resolution) {
  if(box->position.x - box->width >= vg_resolution->x) {
    return true;
  }
  if(box->position.x + box->width <= 0) {
    return true;
  }
  if(box->position.y - box->height >= vg_resolution->y) {
    return true;
  }
  if(box->position.y + box->height <= 0) {
    return true;
  }
  return false;
}

bool check_simple_collision(box_t* b1, box_t* b2) {
  return  b1->position.x              < b2->position.x + b2->width  &&
          b1->position.x + b1->width  > b2->position.x              &&
          b1->position.y              < b2->position.y + b2->height &&
          b1->position.y + b1->height > b2->position.y;
}

//pusher / pushee
void collision_resolution(const box_t* box, box_t* pushable) {

  int pushable_t = pushable->position.y;
  int pushable_b = pushable->position.y + pushable->height;

  int pushable_l = pushable->position.x;
  int pushable_r = pushable->position.x + pushable->width;

  int pushable_ot = pushable->old_position.y;
  int pushable_ob = pushable->old_position.y + pushable->height;

  int pushable_ol = pushable->old_position.x;
  int pushable_or = pushable->old_position.x + pushable->width;

  int box_t = box->position.y;
  int box_b = box->position.y + box->height;

  int box_l = box->position.x;
  int box_r = box->position.x + box->width;

  int box_ot = box->old_position.y;
  int box_ob = box->old_position.y + box->height;

  int box_ol = box->old_position.x;
  int box_or = box->old_position.x + box->width;

  // No collision
  if (pushable_b < box_t || pushable_t > box_b || pushable_l > box_r || pushable_r < box_l)
    return;

  // Bottom collision
  if (pushable_b >= box_t && pushable_ob < box_ot) {
    pushable->position.y = box_t - pushable->height - 1;     // Let colliding box rect push pushable up
  } 
  // Top collision
  else if (pushable_t <= box_b && pushable_ot > box_ob) {
    pushable->position.y = box_b + 1;                      // Let colliding box rect push pushable down
  }
  // Right collision
  else if (pushable_r >= box_l && pushable_or < box_ol) {
    pushable->position.x = box_l - pushable->width - 1;      // Let colliding box rect push pushable left
  }
  // Left collision
  else if (pushable_l <= box_r && pushable_ol > box_or) {
    pushable->position.x = box_r + 1;                      // Let colliding box rect push pushable right
  }
}
