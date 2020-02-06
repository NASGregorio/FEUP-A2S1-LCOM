#include <lcom/lcf.h>
#include <math.h>
#include "lcom/pixmap.h"

#include "gs_hold_your_ground.h"
#include "video.h"
#include "kb_manager.h"
#include "mouse_manager.h"

#include "vector2d.h"
#include "box.h"



const vec2du_t* vg_resolution;
uint16_t max_x;
uint16_t max_y;

xpm_image_t player_sprite;
uint16_t speed = 10;
box_t player;

box_t* rects;
vec2di_t* points;
const vec2di_t* mouse_pos;

size_t click_count = 0;
size_t rect_count = 0;
size_t point_count = 0;

static size_t ticks_since_enter = 0;
static size_t seconds_alive = 0;
static size_t rect_spawn_cooldown = 0;
static size_t rect_spawn_timer = 5;
static size_t counter = 0;

/**
 * @brief Invert rectangle velocities when they hit the edge of the screen
 * 
 */
static void bouncing_rectangles() {
  for(size_t i = 0; i < rect_count; i++)
  {
    rects[i].old_position.x = rects[i].position.x;
    rects[i].old_position.y = rects[i].position.y;
    rects[i].position.x += rects[i].velocity.x;
    rects[i].position.y += rects[i].velocity.y;

    if((rects[i].position.x + rects[i].width) >= max_x) {
      rects[i].position.x = (max_x - rects[i].width);
      rects[i].velocity.x = ((rand()%15) + 1) * -1;
    }
    else if(rects[i].position.x <= 0) {
      rects[i].position.x = 0;
      rects[i].velocity.x = ((rand()%15) + 1);
    }

    if((rects[i].position.y + rects[i].height) >= max_y) {
      rects[i].position.y = (max_y - rects[i].height);
      rects[i].velocity.y = ((rand()%15) + 1) * -1;
    }
    else if(rects[i].position.y <= 0) {
      rects[i].position.y = 0;
      rects[i].velocity.y = ((rand()%15) + 1);
    }

    collision_resolution(&rects[i], &player);
  }
}

/**
 * @brief Generate random sized rectangle at random locations with random velocity
 * 
 */
static void add_random_rectangle() {

  uint16_t random_x = 0;
  uint16_t random_y = 0;

  if(rand() % 100 < 50) {
    random_x = rand() % max_x;
    random_y = (rand() % 100 < 50) ? -100 : max_y + 100; 
  }
  else {
    random_y = rand() % max_y;
    random_x = (rand() % 100 < 50) ? -100 : max_x + 100; 
  }

  uint8_t dir_x = (random_x > max_x) ? -1 : 1;
  uint8_t dir_y = (random_y > max_y) ? -1 : 1;

  rects[rect_count++] = (box_t) {
    .old_position.x = random_x,
    .position.x     = random_x,
    .old_position.y = random_y,
    .position.y     = random_y,
    .width          = (rand() % (100 - 25 + 1)) + 25,
    .height         = (rand() % (100 - 25 + 1)) + 25,
    .velocity.x     = ((rand()%2) + 1) * dir_x,
    .velocity.y     = ((rand()%2) + 1) * dir_y,
  };
}

/**
 * @brief Add random sized rectangle at location x/y with random velocity
 * 
 * @param x x coordinate
 * @param y y coordinate
 */
static void add_rectangle(uint16_t x, uint16_t y) {
  rects[rect_count++] = (box_t) {
    .old_position.x = x,
    .position.x     = x,
    .old_position.y = y,
    .position.y     = y,
    .width          = (rand() % (100 - 25 + 1)) + 25,
    .height         = (rand() % (100 - 25 + 1)) + 25,
    .velocity.x     = (player.position.x - x) / 20,
    .velocity.y     = (player.position.y - y) / 20,
  };
}


static void mouse_logic(game_state_t* gs) {

  mouse_pos = mouse_get_position();

  if(mouse_is_button_down(MS_LB)) {
    points[click_count%16].x = mouse_pos->x;
    points[click_count%16].y = mouse_pos->y;
    click_count++;
    if(point_count < 16) { 
      point_count++;
      if(rect_spawn_timer >= rect_spawn_cooldown) {
        add_rectangle(mouse_pos->x,  mouse_pos->y);
        rect_spawn_timer = 0;
      }
    }
    //printf("Mouse Down LB at %d:%d\n", pos->x, pos->y);
  }
  if(mouse_is_button_up(MS_RB)) {
    point_count = 0;
    //printf("Mouse Up RB at %d:%d\n", pos->x, pos->y);
  }
}

static void kb_logic(game_state_t* gs) {
  
  if(kb_is_key_down(KEY_ESC)) {
    gs->next_state = STATE_MENU;
    return;
  }

  if(kb_is_key_held(KEY_W)) {
    player.velocity.y -= speed;
  }
  else if(kb_is_key_held(KEY_S)) {
    player.velocity.y += speed;
  }

  if(kb_is_key_held(KEY_D)) {
    player.velocity.x += speed;
  }
  else if(kb_is_key_held(KEY_A)) {
    player.velocity.x -= speed;
  }
}

static void gs_draw(game_state_t* gs) {

  // Draw game state in frame buffer
  for (size_t i = 0; i < point_count; i++) {
    vg_draw_rectangle(points[i].x, points[i].y, 24, 24, (i+1)%256);
  }

  for (size_t i = 0; i < rect_count; i++) {
    vg_draw_rectangle(rects[i].position.x, rects[i].position.y, rects[i].width, rects[i].height, (i+1)%16);
  }

  vg_draw_xpm(&player_sprite, player.position.x, player.position.y);
  vg_draw_rectangle(mouse_pos->x, mouse_pos->y, 16, 16, 63);

  // Copy frame buffer to GPU Buffer
  vg_copy_buffer();

  // Set frame buffer back to black (background colour)
  for (size_t i = 0; i < point_count; i++) {
    vg_draw_rectangle(points[i].x, points[i].y, 24, 24, 0);
  }

  for (size_t i = 0; i < rect_count; i++) {
    vg_draw_rectangle(rects[i].position.x, rects[i].position.y, rects[i].width, rects[i].height, 0);
  }

  vg_clear_xpm(&player_sprite, player.position.x, player.position.y);
  vg_draw_rectangle(mouse_pos->x, mouse_pos->y, 16, 16, 0);
}

static void gs_update(game_state_t* gs) {

  ticks_since_enter++;
  if((++counter) % 60 == 0) {
    seconds_alive++;
    rect_spawn_timer++;
  }

  // Reset player velocity
  player.velocity.x = 0;
  player.velocity.y = 0;

  if(check_box_out_of_screen(&player, vg_resolution)) {
    snprintf(gs->error_msg, 256, "Seconds alive: %zu", seconds_alive);

    gs->is_game_running = false;
  }

  kb_logic(gs);
  mouse_logic(gs);

  // Cache last frame position
  player.old_position.x = player.position.x;
  player.old_position.y = player.position.y;

  // Add this frame's velocity to current position
  player.position.x += player.velocity.x;
  player.position.y += player.velocity.y;

  // Moving rectangles' logic
  bouncing_rectangles();
}

static void gs_enter(game_state_t* gs) {

  // Init video mode
  uint8_t* vram_addr = vg_init(0x103); //0x114 //0x114
  if(vram_addr == NULL) {
    snprintf(gs->error_msg, 256, "GS1: VG_INIT_ERROR");
    gs->is_game_running = false;
    return;
  }

  vg_clear_screen();


  srand((unsigned) time(NULL));

  point_count = 0;
  rect_count = 0;
  click_count = 0;

  vg_resolution = vg_get_resolution();
  max_x = vg_resolution->x;
  max_y = vg_resolution->y;

  mouse_set_boundaries(vg_resolution);

  vg_load_xpm(penguin, &player_sprite);

  player = (box_t) {
    .old_position.x = vg_resolution->x/2,
    .position.x = vg_resolution->x/2,
    .old_position.y = vg_resolution->y/2,
    .position.y = vg_resolution->y/2,
    .width     = player_sprite.width,
    .height     = player_sprite.height,
  };

  points = malloc(256 * sizeof(vec2di_t));
  rects = malloc(16 * sizeof(box_t));

  add_random_rectangle();

  add_random_rectangle();
}

static void gs_exit(game_state_t* gs) {
  free(points);
  free(rects);

  // Switch back to text mode
  vg_destroy();
}

void switch_to_gs_hold(game_state_t* gs) {
  gs->enter = gs_enter;
  gs->update = gs_update;
  gs->draw = gs_draw;
  gs->exit = gs_exit;
}

