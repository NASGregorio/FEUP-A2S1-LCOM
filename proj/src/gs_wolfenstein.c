#include <lcom/lcf.h>
#include <stdbool.h>

#include <math.h>

#include "gs_wolfenstein.h"
#include "video.h"
#include "kb_manager.h"
#include "mouse_manager.h"

#include "vector2d.h"

#define SIGN(x) ( ( (x) < 0 ) ? -1 : 1 )

/**
 * @brief Top down map representation
 * 
 */
typedef const struct {
  const char* map;        /**< Pointer to char map array */
  size_t width;           /**< Map width */
  size_t height;          /**< Map height */
  size_t visibility_dst;  /**< Falloff distance for depth calculation */
} map_t;

map_t map1 = (map_t) {
  .map = 
  "################"
  "#..............#"
  "#..............#"
  "#..............#"
  "#..............#"
  "#..............#"
  "#..............#"
  "#..............#"
  "#..............#"
  "#..............#"
  "#..............#"
  "#..............#"
  "#..............#"
  "#..............#"
  "#..............#"
  "#######XX#######",
  .width = 16,
  .height = 16,
  .visibility_dst = 8,
};

map_t map2 = (map_t) {
  .map = 
  "#######XX#######"
  "#..............#"
  "#..##########..#"
  "#..#........#..#"
  "#..###......#..#"
  "#...........#..#"
  "#..##..#######.#"
  "#..#.........#.#"
  "#..#.........#.#"
  "#..#####.....#.#"
  "#......##..#...#"
  "#.......#..#...#"
  "#.......#..###.#"
  "#..............#"
  "#..............#"
  "################",
  .width = 16,
  .height = 16,
  .visibility_dst = 8,
};

map_t map3 = (map_t) {
  .map = 
  "#######################"
  "#.........#...........#"
  "#......##.#...........#"
  "#......##.#.....#######"
  "#####.....#...........#"
  "#.........#...........#"
  "##.######.....###.#####"
  "##.#......##.##.#.#...#"
  "##.##.##..#....##.#...#"
  "####...#..#...##..#...#"
  "#..#.#.#..#.###.#.#####"
  "##...#......#......#..#"
  "#########.###..#.....##"
  "#...#.....#..#...######"
  "#...#.#####......#.##.#"
  "#...#.#...########..#.#"
  "#..##.###.#.........#.#"
  "#.##......#...##..#.#.#"
  "#.#..##...#.#..#..#...#"
  "#...#####.........#.#.#"
  "#######################",
  .width = 21,
  .height = 23,
  .visibility_dst = 8,
};


const map_t* current_map = &map1;

const vec2du_t* vg_resolution;

vec2df_t player_pos = (vec2df_t) {
  .x = 8.0f,
  .y = 8.0f,
};

float player_heading = 3.14159/2.0;
float player_fov = 3.14159/4.0;

int column_count = 200;
int column_width = 0;

vec2df_t temp_dir;
vec2di_t curr_tile;
vec2df_t tile_offset;

float tan_theta;
float one_over_tan_theta;

vec2df_t horizontal_intercept;
vec2df_t vertical_intercept;
vec2df_t vec_intercept;
vec2di_t tile_step;

/**
 * @brief Calculate distance to wall on the map grid for the given angle
 * 
 * @param theta Player angle
 * @param column_type Type of column
 * @return float 
 */
static float check_ray(float theta, int* column_type) {
  temp_dir.x = cosf(theta);
  temp_dir.y = sinf(theta);

  curr_tile.x = (int)floorf(player_pos.x);
  curr_tile.y = (int)floorf(player_pos.y);

  tile_offset.x = player_pos.x - curr_tile.x;
  tile_offset.y = player_pos.y - curr_tile.y;
  
  tan_theta = atan2f(temp_dir.y, temp_dir.x);

  tan_theta = temp_dir.y / temp_dir.x;
  one_over_tan_theta = 1 / tan_theta;

  horizontal_intercept.x = player_pos.x;
  horizontal_intercept.y = player_pos.y;
  horizontal_intercept.y += (temp_dir.y >= 0) ? (1 - tile_offset.y) : -tile_offset.y;
  horizontal_intercept.x += (temp_dir.y >= 0) ? (1 - tile_offset.y) * one_over_tan_theta : -tile_offset.y * one_over_tan_theta;

  vertical_intercept.x = player_pos.x;
  vertical_intercept.y = player_pos.y;
  vertical_intercept.x += (temp_dir.x >= 0) ? (1 - tile_offset.x) : -tile_offset.x;
  vertical_intercept.y += (temp_dir.x >= 0) ? (1 - tile_offset.x) * tan_theta : -tile_offset.x * tan_theta;

  tile_step.x = SIGN(temp_dir.x);
  tile_step.y = SIGN(temp_dir.y);

  while( 1 )
  {
    // < no 1º e 2º quadrantes
    // > no 3º e 4º quadrantes

    if(((tile_step.y == 1) ? horizontal_intercept.y <= vertical_intercept.y : horizontal_intercept.y > vertical_intercept.y))
    {
      curr_tile.y += tile_step.y;

      if(strchr("#X", current_map->map[curr_tile.y + curr_tile.x * current_map->width]) != NULL)
      {
        if(current_map->map[curr_tile.y + curr_tile.x * current_map->width] == 'X')
          *column_type = 2;
        else
          *column_type = 1;

        vec_intercept = horizontal_intercept;
        break;
      }

      horizontal_intercept.y += tile_step.y;
      horizontal_intercept.x += one_over_tan_theta * tile_step.y;
    }
    else
    {
      curr_tile.x += tile_step.x;
      if(strchr("#X", current_map->map[curr_tile.y + curr_tile.x * current_map->width]) != NULL)
      {
        if(current_map->map[curr_tile.y + curr_tile.x * current_map->width] == 'X')
          *column_type = 2;
        else
          *column_type = 1;

        vec_intercept = vertical_intercept;
        break;
      }
      vertical_intercept.x += tile_step.x;
      vertical_intercept.y += tan_theta * tile_step.x;
    }
  }

  float dst = (vec_intercept.x-player_pos.x) * cosf(player_heading) + (vec_intercept.y-player_pos.y) * sinf(player_heading);

  return dst;
}

/**
 * @brief Change value from interval [value_min, value_max] to [b_min, b_max]
 * 
 * @param value     value to change
 * @param value_min value's min range
 * @param value_max value's max range
 * @param b_min     byte min range
 * @param b_max     byte max range
 * @return uint8_t  value in byte range
 */
static uint8_t convert_to_byte_range(float value, float value_min, float value_max, uint8_t b_min, uint8_t b_max) {

  float lerp_01 = (value - value_min) / (value_max - value_min);
  uint8_t byte = (uint8_t) ( lerp_01 * (b_max - b_min) + b_min );

  return byte;
}

/**
 * @brief Draw ceiling with color gradient
 * 
 * @param x Starting in x
 * @param width Width of ceiling
 */
static void draw_ceiling_column(uint16_t x, uint16_t width) {

  uint8_t lerp_r = 0;
  uint8_t lerp_g = 0;
  uint8_t lerp_b = 0;
  uint32_t color = 0;

  for (size_t j = 0; j < vg_resolution->y/3; j++)
  {
    lerp_g = convert_to_byte_range(j, 0, vg_resolution->y/3.0, 0x10, 0x00);
    lerp_b = convert_to_byte_range(j, 0, vg_resolution->y/3.0, 0x20, 0x00);
    color = (lerp_r << 8*2) + (lerp_g << 8) + (lerp_b);

    for (size_t i = x; i < x + width; i++)
      vg_draw_pixel(i, j, color);
  }
}


/**
 * @brief Draw floor with color gradient
 * 
 * @param x Starting in x
 * @param width Width of floor
 */
static void draw_floor_column(uint16_t x, uint16_t width) {

  uint8_t lerp_r = 0;
  uint8_t lerp_g = 0;
  uint8_t lerp_b = 0;
  uint32_t color = 0;

  for (size_t j = vg_resolution->y-1; j >= 2*vg_resolution->y/3; j--)
  {
    lerp_r = convert_to_byte_range(j, vg_resolution->y-1, 2*vg_resolution->y/3, 0x10, 0x00);
    lerp_g = convert_to_byte_range(j, vg_resolution->y-1, 2*vg_resolution->y/3, 0x10, 0x00);
    color = (lerp_r << 8*2) + (lerp_g << 8) + (lerp_b);

    for (size_t i = x; i < x + width; i++)
      vg_draw_pixel(i, j, color);
  }
}

/**
 * @brief Sweep player FOV, checking distance to the wall at every angle increment
 * Draw rectangle with height and color based on the distance.
 * 
 */
static void draw_view() {
  float angle_step = player_fov / column_count;
  float start_angle = (player_heading + player_fov/2);
  
  uint8_t high = 0x80;
  uint8_t low = 0x00;
  uint8_t high_b = 0x00;
  uint8_t lerp = low;
  uint8_t lerp_b = high_b;
  uint32_t color = lerp;

  float dst = 0;
  int ceiling_height = 0;
  int floor_height = 0;

  for (int i = 0; i < column_count; i++)
  {
    int column_type = 0;
    dst = check_ray( start_angle - angle_step * i, &column_type );

    high_b = (column_type == 1) ? high : 0x00;

    ceiling_height = MAX(0, (vg_resolution->y / 2.0) - (vg_resolution->y / dst) ); // center of the screen minus "inverse dst to the wall"
    floor_height = vg_resolution->y - ceiling_height;

    dst = MIN(dst, current_map->visibility_dst);

    lerp = low + (high - low) * (1-dst/current_map->visibility_dst);
    lerp_b = low + (high_b - low) * (1-dst/current_map->visibility_dst);
    color = (lerp << 8*2) + (lerp << 8*1) + lerp_b;

    if(ceiling_height != 0) {
      draw_ceiling_column((i*column_width), column_width);
      draw_floor_column((i*column_width), column_width);
    }

    vg_draw_rectangle((i*column_width), ceiling_height, column_width, floor_height - ceiling_height, color);
  }
}


static void gs_draw(game_state_t* gs) {
  vg_clear_screen();
  
  draw_view();

  vg_copy_buffer();
}

static void gs_update(game_state_t* gs) {

  if(kb_is_key_down(KEY_ESC)) {
    gs->next_state = STATE_MENU;
    return;
  }

  if(kb_is_key_down(KEY_1)) {
    current_map = &map1;
  }
  else if(kb_is_key_down(KEY_2)) {
    current_map = &map2;
  }
  else if(kb_is_key_down(KEY_3)) {
    current_map = &map3;
  }

  float stepX = cosf(player_heading);
  float stepY = sinf(player_heading);

  float sqrtStep = sqrt(stepX*stepX + stepY*stepY);

  stepX /= sqrtStep;
  stepY /= sqrtStep;

  if(kb_is_key_held(KEY_W)) {
    player_pos.y += 0.1f * stepY;
    player_pos.x += 0.1f * stepX;

    if(current_map->map[ (int)player_pos.y + (int)player_pos.x * current_map->width] == '#') {
      player_pos.y -= 0.1f * stepY;
      player_pos.x -= 0.1f * stepX;
    }
  }
  else if(kb_is_key_held(KEY_S)) {
    player_pos.y -= 0.1f * stepY;
    player_pos.x -= 0.1f * stepX;

    if(current_map->map[ (int)player_pos.y + (int)player_pos.x * current_map->width] == '#') {
      player_pos.y += 0.1f * stepY;
      player_pos.x += 0.1f * stepX;
    }
  }
  if(kb_is_key_held(KEY_D)) {
    player_heading -= 0.05f;
  }
  else if(kb_is_key_held(KEY_A)) {
    player_heading += 0.05f;
  }
}

static void gs_enter(game_state_t* gs) {

  // Init video mode
  uint8_t* vram_addr = vg_init(0x115); //0x105
  if(vram_addr == NULL) {
    snprintf(gs->error_msg, 256, "GS1: VG_INIT_ERROR");
    gs->is_game_running = false;
    return;
  }

  vg_clear_screen();

  vg_resolution = vg_get_resolution();

  column_width = vg_resolution->x / column_count;

  current_map = &map1;
}

static void gs_exit(game_state_t* gs) {
  // Switch back to text mode
  vg_destroy();
}

void switch_to_gs_wolfenstein(game_state_t* gs) {
  gs->enter = gs_enter;
  gs->update = gs_update;
  gs->draw = gs_draw;
  gs->exit = gs_exit;
}
