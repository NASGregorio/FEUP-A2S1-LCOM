#include <lcom/lcf.h>
#include <math.h>

#include "gs_menu.h"

#include "video.h"
#include "kb_manager.h"
#include "mouse_manager.h"
#include "rtc.h"

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
#define FONT_DOT 36
#define FONT_SLASH 37

const vec2du_t* vg_resolution;
uint16_t max_x;
uint16_t max_y;

const vec2di_t* mouse_pos;

bitmap_t* menu;
bitmap_t* cursor;
bitmap_t* font_spritesheet;
bitmap_t* howto;

vec2du_t font_offset;
vec2du_t font_size = (vec2du_t) {
  .x = FONT_SIZE_X,
  .y = FONT_SIZE_Y,
};

box_t s1;
box_t s2;
box_t s3;
box_t s4;

static bool display_how_to = false;

uint8_t sec;
uint8_t min;
uint8_t hour;

uint8_t day;
uint8_t month;
uint8_t year;


static void mouse_logic(game_state_t* gs) {
  mouse_pos = mouse_get_position();

  if(mouse_is_button_down(MS_LB)) {
    if(mouse_is_inside_area(&s1)) {
      gs->next_state = STATE_HOLD;
    }
    else if(mouse_is_inside_area(&s2)) {
      gs->next_state = STATE_RUNNER;
    }
    else if(mouse_is_inside_area(&s3)) {
      gs->next_state = STATE_WOLFENSTEIN;
    }

    else if(mouse_is_inside_area(&s4)) {
      display_how_to = !display_how_to;
    }
  }
}

/**
 * @brief Displays a character based on a spritesheet
 * 
 * @param char_type Type of character
 * @param x X position on spritesheet
 * @param y Y position on spritesheet
 */
static void display_char(uint8_t char_type, int x, int y) {

  font_offset.x = FONT_SPACING_X + (char_type % 10) * (FONT_SPACING_X + FONT_SIZE_X);
  font_offset.y = FONT_SPACING_Y + (char_type / 10) * (FONT_SPACING_Y + FONT_SIZE_Y);

  vg_draw_bitmap_area(font_spritesheet, x, y, &font_offset, &font_size);
}

/**
 * @brief Displays a character based on a spritesheet
 * 
 * @param number Number to be displayed
 * @param x X position on spritesheet
 * @param y Y position on spritesheet
 */
static void display_number(size_t number, int x, int y) {
  int offset = 0;
  int num = 0;

  if(number == 0) {
    display_char(FONT_0, x, y);
    return;
  }

  while (number > 0)
  {
    num = (number % 10);
    if(num == 0)
      display_char(FONT_0, x-offset, y);
    else
      display_char((number % 10) - 1,x-offset, y);
    number /= 10;
    offset += 24;
  }
}

/**
 * @brief Displays time given by RTC at screen position x/y
 * 
 * @param x X position 
 * @param y Y position
 */
static void display_time(int x, int y) {

  int offset = x;

  if(hour < 10)
    display_char(FONT_0, offset, y);

  offset += 25;
  display_number(hour, offset, y);

  offset += 20;
  display_char(FONT_DOT, offset, y);

  offset += 20;
  if(min < 10)
    display_char(FONT_0, offset, y);
  
  offset += 25;
  display_number(min, offset, y);

  offset += 20;
  display_char(FONT_DOT, offset, y);

  offset += 20;
  if(sec < 10)
    display_char(FONT_0, offset, y);

  offset += 25;
  display_number(sec, offset, y);
}

/**
 * @brief Displays date given by RTC at screen position x/y
 * 
 * @param x X position 
 * @param y Y position
 */
static void display_date(int x, int y) {

  int offset = x;

  if(day < 10)
    display_char(FONT_0, offset, y);

  offset += 25;
  display_number(day, offset, y);

  offset += 20;
  display_char(FONT_SLASH, offset, y);

  offset += 20;
  if(month < 10)
    display_char(FONT_0, offset, y);
  
  offset += 25;
  display_number(month, offset, y);

  offset += 20;
  display_char(FONT_SLASH, offset, y);

  offset += 20;
  if(year < 10)
    display_char(FONT_0, offset, y);

  offset += 25;
  display_number(year, offset, y);
}

static void gs_draw(game_state_t* gs) {

  if(display_how_to) {
    vg_draw_bitmap_by_line(howto, 0, 0);
  }
  else {
    vg_draw_bitmap_by_line(menu, 0, 0);

    display_time(15, 60);
    display_date(15, 90);
  }

  vg_draw_bitmap(cursor, mouse_pos->x, mouse_pos->y);

  // Copy frame buffer to GPU Buffer
  vg_copy_buffer();
}

static void gs_update(game_state_t* gs) {
  mouse_logic(gs);

  rtc_get_param(RTC_REG_SEC, &sec);
  rtc_get_param(RTC_REG_MIN, &min);
  rtc_get_param(RTC_REG_HOUR, &hour);

  rtc_get_param(RTC_REG_DMOT, &day);
  rtc_get_param(RTC_REG_MOT, &month);
  rtc_get_param(RTC_REG_YEAR, &year);

  if(kb_is_key_down(KEY_ESC))
    gs->is_game_running = false;
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

  menu = vg_load_bitmap("/home/lcom/labs/proj/src/bitmaps/menu.bmp");
  howto = vg_load_bitmap("/home/lcom/labs/proj/src/bitmaps/howto.bmp");
  cursor = vg_load_bitmap("/home/lcom/labs/proj/src/bitmaps/cursor.bmp");
  font_spritesheet = vg_load_bitmap("/home/lcom/labs/proj/src/bitmaps/ui_font.bmp");


  s1.position.x = 43;
  s1.position.y = 376;
  s1.width = 222;
  s1.height = 193;

  s2.position.x = 280;
  s2.position.y = 440;
  s2.width = 273;
  s2.height = 149;

  s3.position.x = 609;
  s3.position.y = 311;
  s3.width = 163;
  s3.height = 258;

  s4.position.x = 400;
  s4.position.y = 365;
  s4.width = 170;
  s4.height = 60;

}

static void gs_exit(game_state_t* gs) {

  // Cleanup
  vg_destroy_bitmap(menu);
  vg_destroy_bitmap(howto);
  vg_destroy_bitmap(cursor);
  vg_destroy_bitmap(font_spritesheet);

  // Switch back to text mode
  vg_destroy();

}

void switch_to_gs_menu(game_state_t* gs) {
  gs->enter = gs_enter;
  gs->update = gs_update;
  gs->draw = gs_draw;
  gs->exit = gs_exit;
}
