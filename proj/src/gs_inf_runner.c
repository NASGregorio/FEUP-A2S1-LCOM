#include <lcom/lcf.h>
#include <math.h>

#include "gs_inf_runner.h"
#include "video.h"
#include "kb_manager.h"
#include "mouse_manager.h"

#include "vector2d.h"
#include "box.h"

#define FONT_SPACING_X 4
#define FONT_SPACING_Y 4
#define FONT_SIZE_X 20
#define FONT_SIZE_Y 20

#define FONT_1 0
#define FONT_2 1
#define FONT_3 2
#define FONT_4 3
#define FONT_5 4
#define FONT_6 5
#define FONT_7 6
#define FONT_8 7
#define FONT_9 8
#define FONT_0 9
#define FONT_A 10

const vec2du_t* vg_resolution;
uint16_t max_x;
uint16_t max_y;

const vec2di_t* mouse_pos;

bitmap_t* bg;
bitmap_t* road;
bitmap_t* cursor;
bitmap_t* player_sprite;
bitmap_t* enemy_sprite;
bitmap_t* fuel_sprite;
bitmap_t* font_spritesheet;

int road_pos_y  = 0;
int road_speed  = 5;
int road_height = 0;
uint32_t road_width = 0;

box_t player;
static uint16_t player_speed = 4;
static uint16_t total_speed;
vec2du_t speed_font_offset;
vec2du_t speed_font_size = (vec2du_t) {
  .x = FONT_SIZE_X,
  .y = FONT_SIZE_Y,
};

box_t fuel;
uint16_t fuel_boost = 0;
static int fuel_spawn_cooldown = 5;
static int fuel_spawn_timer = 0;
static int fuel_decrease_timer = -5;
#define LOW_FUEL 50
#define MID_FUEL 90
#define FULL_FUEL 165
static uint16_t fuel_bar_width = LOW_FUEL;

box_t* enemies;
uint8_t enemy_count = 16;
uint8_t enemy_next = 0;
char enemies_visible[16] = "NNNNNNNNNNNNNNNN";
#define ENEMY_START_HEIGHT (-20)

static size_t ticks_since_enter = 0;
static size_t seconds_alive = 0;
static size_t counter = 0;

static int enemy_spawn_cooldown = 3;
static int enemy_spawn_timer = 0;
static int spawns_per_cooldown = 3;
static int spawns_used = 0;



/**
 * @brief Create array of boxes to track enemies on screen
 * 
 */
static void init_enemies() {

  enemies = malloc(enemy_count * sizeof(box_t));

  uint16_t width = enemy_sprite->info->bV5Width;
  uint16_t height = enemy_sprite->info->bV5Height;

  for (size_t i = 0; i < enemy_count; i++)
  {
    enemies[i] = (box_t) {
      .old_position.x = 0,
      .position.x     = 0,
      .old_position.y = ENEMY_START_HEIGHT,
      .position.y     = ENEMY_START_HEIGHT,
      .width          = width,
      .height         = height,
      .velocity.x     = 0,
      .velocity.y     = 0,
    };
  }
}

/**
 * @brief Relocate enemy to top of screen at x coordinate and add downwards velocity
 * 
 * @param x x coordinate
 */
static void spawn_enemy(int x) {
  enemies[enemy_next].position.x = x;  
  enemies[enemy_next].velocity.y = road_speed;
  enemies_visible[enemy_next++] = 'Y';

  if(enemy_next >= enemy_count)
    enemy_next = 0;
}

/**
 * @brief Place fuel above the screen with downwards velocity at a random x coordinate (inside the road)
 * 
 */
static void add_random_fuel() {

  uint16_t random_x = (rand() % (170 - 420 + 1)) + 170;

  fuel.position.x = random_x;
  fuel.old_position.x = random_x;
  fuel.old_position.y = ENEMY_START_HEIGHT;
  fuel.position.y = ENEMY_START_HEIGHT;
  fuel.velocity.y = road_speed;
}

/**
 * @brief Draw selected char from font spritesheet at location x/y
 * 
 * @param char_type Selected char
 * @param x         x coordinate
 * @param y         y coordinate
 */
static void display_char(uint8_t char_type, int x, int y) {

  speed_font_offset.x = FONT_SPACING_X + (char_type % 10) * (FONT_SPACING_X + FONT_SIZE_X);
  speed_font_offset.y = FONT_SPACING_Y + (char_type / 10) * (FONT_SPACING_Y + FONT_SIZE_Y);

  vg_draw_bitmap_area(font_spritesheet, x, y, &speed_font_offset, &speed_font_size);
}

/**
 * @brief Draw player speed
 * 
 */
static void display_speed() {
  display_char(total_speed - 1, 691, 244);
}

/**
 * @brief Draw player socre
 * 
 */
static void display_score() {
  size_t sec = seconds_alive*10;
  int offset = 0;
  int num = 0;
  while (sec > 0)
  {
    num = (sec % 10);
    if(num == 0)
      display_char(9, 745-offset, 62);
    else
      display_char((sec % 10) - 1, 745-offset, 62);
    sec /= 10;
    offset += 24;
  }
}


static void kb_logic(game_state_t* gs) {

  if(kb_is_key_down(KEY_ESC)) {
    gs->next_state = STATE_MENU;
    return;
  }

  if(kb_is_key_held(KEY_W)) {
    player.velocity.y -= total_speed;
  }
  else if(kb_is_key_held(KEY_S)) {
    player.velocity.y += total_speed;
  }

  if(kb_is_key_held(KEY_D)) {
    player.velocity.x += total_speed;
  }
  else if(kb_is_key_held(KEY_A)) {
    player.velocity.x -= total_speed;
  }
}

static void mouse_logic(game_state_t* gs) {
  mouse_pos = mouse_get_position();

  if(mouse_is_button_down(MS_LB) && spawns_used < spawns_per_cooldown) {
    spawn_enemy(mouse_pos->x);
    spawns_used++;
    if(spawns_used >= spawns_per_cooldown) {
      enemy_spawn_timer = enemy_spawn_cooldown;
    }
  }
}



static void gs_draw(game_state_t* gs) {

  vg_draw_bitmap_by_line(bg, 0, 0);
  vg_draw_bitmap_by_line(road, 165, road_pos_y);
  vg_draw_bitmap_by_line(road, 165, road_pos_y-road_height);

  for (size_t i = 0; i < enemy_count; i++) {

    if(enemies_visible[i] == 'N')
      continue;

    vg_draw_bitmap(enemy_sprite, enemies[i].position.x,  enemies[i].position.y);
  }

  vg_draw_bitmap(fuel_sprite, fuel.position.x, fuel.position.y);

  vg_draw_bitmap(player_sprite, player.position.x, player.position.y);

  vg_draw_rectangle(165, 0, road_width, 12, 0);
  vg_draw_rectangle(165, max_y-12, road_width, 12, 0);

  vg_draw_rectangle(620, 150, fuel_bar_width, 24, 0xFC08);

  vg_draw_bitmap(cursor, mouse_pos->x, mouse_pos->y);

  display_speed();

  display_score();

  // Copy frame buffer to GPU Buffer
  vg_copy_buffer();
}

static void gs_update(game_state_t* gs) {

  ticks_since_enter++;
  if((++counter) % 60 == 0) {

    seconds_alive++;
    fuel_spawn_timer++;
    fuel_decrease_timer++;

    if(fuel_spawn_timer >= fuel_spawn_cooldown) {
      fuel_spawn_timer = 0;
      add_random_fuel();

      if(fuel_boost > 0 && fuel_decrease_timer >= fuel_spawn_cooldown * 2) {
        fuel_decrease_timer = 0;
        fuel_boost--;
      }
    }

    if(spawns_used == spawns_per_cooldown) {
      enemy_spawn_timer--;

      if(enemy_spawn_timer <= 0) {
        spawns_used = 0;
      }
    }
  }

  // Reset player velocity
  player.velocity.x = 0;
  player.velocity.y = 0;
  total_speed = player_speed + fuel_boost;

  kb_logic(gs);
  mouse_logic(gs);

  // Scroll background
  road_pos_y += road_speed;
  if(road_pos_y > road_height)
    road_pos_y = 0;

  // Animate enemies down screen
  for (size_t i = 0; i < enemy_count; i++) {

    if(enemies_visible[i] == 'N')
      continue;

    if(check_simple_collision(&player, &enemies[i])) {
      snprintf(gs->error_msg, 256, "Seconds alive: %zu", seconds_alive);

      gs->is_game_running = false;
    }

    enemies[i].position.y += road_speed;

    if(enemies[i].position.y > max_y+10) {
      enemies[i].position.y = ENEMY_START_HEIGHT;
      enemies_visible[i] = 'N';
    }
  }

  if(check_simple_collision(&player, &fuel)) {
    fuel.position.y = max_y + 10;
    if(fuel_boost < 3) {
      fuel_boost++;
    }
  }

  switch (fuel_boost)
  {
    case 0:
      fuel_bar_width = LOW_FUEL;
      break;
    case 1:
      fuel_bar_width = MID_FUEL;
      break;
    case 3:
      fuel_bar_width = FULL_FUEL;
      break;
    default:
      fuel_bar_width = FULL_FUEL;
      break;
  }

  fuel.position.y += fuel.velocity.y;

  // Cache last frame position
  player.old_position.x = player.position.x;
  player.old_position.y = player.position.y;

  // Add this frame's velocity to current position
  player.position.x += player.velocity.x;
  player.position.y += player.velocity.y;

  player.position.y = MAX(10, player.position.y);
  player.position.y = MIN(max_y-player.height-12, player.position.y);

  player.position.x = MIN(420, player.position.x);
  player.position.x = MAX(170, player.position.x);
}

static void gs_enter(game_state_t* gs) {

  // Init video mode
  uint8_t* vram_addr = vg_init(0x114); //0x114 //0x114
  if(vram_addr == NULL) {
    snprintf(gs->error_msg, 256, "GS1: VG_INIT_ERROR");
    gs->is_game_running = false;
    return;
  }
  vg_clear_screen();

  vg_resolution = vg_get_resolution();
  max_x = vg_resolution->x;
  max_y = vg_resolution->y;

  mouse_set_boundaries(vg_resolution);

  bg = vg_load_bitmap("/home/lcom/labs/proj/src/bitmaps/background.bmp");
  road = vg_load_bitmap("/home/lcom/labs/proj/src/bitmaps/road.bmp");
  cursor = vg_load_bitmap("/home/lcom/labs/proj/src/bitmaps/cursor.bmp");
  font_spritesheet = vg_load_bitmap("/home/lcom/labs/proj/src/bitmaps/ui_font.bmp");
  player_sprite = vg_load_bitmap("/home/lcom/labs/proj/src/bitmaps/player.bmp");
  enemy_sprite = vg_load_bitmap("/home/lcom/labs/proj/src/bitmaps/enemy.bmp");
  fuel_sprite = vg_load_bitmap("/home/lcom/labs/proj/src/bitmaps/fuel.bmp");

  srand((unsigned) time(NULL));
  road_height = (int)(road->info->bV5Height);
  road_width = road->info->bV5Width;

  player = (box_t) {
    .old_position.x = 100,
    .position.x = 100,
    .old_position.y = 100,
    .position.y = 100,
    .width     = player_sprite->info->bV5Width,
    .height     = player_sprite->info->bV5Height,
  };

  fuel = (box_t) {
    .old_position.x = 100,
    .position.x     = 100,
    .old_position.y = ENEMY_START_HEIGHT,
    .position.y     = ENEMY_START_HEIGHT - fuel_sprite->info->bV5Height,
    .width          = fuel_sprite->info->bV5Width,
    .height         = fuel_sprite->info->bV5Height,
    .velocity.x     = 0,
    .velocity.y     = 0,
  };

  init_enemies();

}

static void gs_exit(game_state_t* gs) {

  // Cleanup
  vg_destroy_bitmap(bg);
  vg_destroy_bitmap(road);
  vg_destroy_bitmap(cursor);
  vg_destroy_bitmap(font_spritesheet);
  vg_destroy_bitmap(player_sprite);
  vg_destroy_bitmap(enemy_sprite);
  vg_destroy_bitmap(fuel_sprite);

  free(enemies);

  // Switch back to text mode
  vg_destroy();

}

void switch_to_gs_runner(game_state_t* gs) {
  gs->enter = gs_enter;
  gs->update = gs_update;
  gs->draw = gs_draw;
  gs->exit = gs_exit;
}
