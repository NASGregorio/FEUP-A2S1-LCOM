#ifndef __VIDEO_H__
#define __VIDEO_H__

#include <stdint.h>
#include "vector2d.h"


#define VBE_VIDEO_INTERRUPT 0x10
#define VBE_SET_MODE 0x4F02
#define VBE_GET_MODE_INFO 0x4F01

#define VBE_FUNC_CALL_FAIL 0x01
#define VBE_FUNC_NOT_SUP_IN_HW 0x02
#define VBE_FUNC_INVAL_IN_CURR_MODE 0x03

#define VBE_FUNC_CALL_FAIL_ERROR 1
#define VBE_FUNC_NOT_SUP_IN_HW_ERROR 2
#define VBE_FUNC_INVAL_IN_CURR_MODE_ERROR 3
#define VBE_FUNC_NOT_SUPPORTED 5
#define VBE_FUNKNOWN_ERROR 6
#define VBE_GET_MODE_INFO_ERROR 7
#define SYS_INT86_FAIL 8

#define VG_INIT_ERROR 9
#define VG_OUT_OF_BOUNDS 10
#define VG_XPM_LOAD_FAIL 11

#define MAX_16_BIT 65535

typedef union {
  uint32_t color;
  uint8_t colors[4];
} color_t;

typedef enum {
  CM_Indexed = 4,
  CM_Direct = 6,
  CM_Other = 0
} color_mode_t;


/**
 * @brief Video mode information
 * 
 */
typedef struct {
  uint8_t* vram_addr;
  size_t vram_size;
  vec2du_t res;
  uint8_t bits_per_pixel;
  uint8_t bytes_per_pixel;
  color_mode_t color_mode;
  uint8_t RedMaskSize;        
  uint8_t GreenMaskSize;
  uint8_t BlueMaskSize;
} video_info_t;


/**
 * @brief Bitmap file header
 * 
 */
typedef struct {
  uint16_t type; 
  uint32_t size;
  uint32_t reserved;
  uint32_t offset;
} __attribute__((__packed__)) bitmap_file_header_t;

/**
 * @brief Bitmap information header
 * 
 */
typedef struct BITMAPV5HEADER { // uint32_t, uint32_t: 4 bytes uint16_t: 2 bytes
  uint32_t  bV5Size; // this header size
  uint32_t  bV5Width;
  uint32_t  bV5Height; // If negative, first row is top
  uint16_t  bV5Planes;
  uint16_t  bV5BitCount; // Bits per pixel
  uint32_t  bV5Compression;
  uint32_t  bV5SizeImage;
  uint32_t  bV5XPelsPerMeter;
  uint32_t  bV5YPelsPerMeter;
  uint32_t  bV5ClrUsed;
  uint32_t  bV5ClrImportant;
  uint32_t  bV5RedMask;
  uint32_t  bV5GreenMask;
  uint32_t  bV5BlueMask;
  uint32_t  bV5AlphaMask;
  uint32_t  bV5CSType;
  uint8_t   bV5Endpoints[36]; // 36 bytes
  uint32_t  bV5GammaRed;
  uint32_t  bV5GammaGreen;
  uint32_t  bV5GammaBlue;
  uint32_t  bV5Intent;
  uint32_t  bV5ProfileData;
  uint32_t  bV5ProfileSize;
  uint32_t  bV5Reserved;
} bitmap_info_header_t;

/**
 * @brief Bitmap structure
 * 
 */
typedef struct {
    bitmap_info_header_t* info; /**< Pointer to bitmap information header */
    uint8_t* data;              /**< Pointer to bitmap color data */
} bitmap_t;

/**
 * @brief Get mode info from vbe
 * 
 * @param mode Mode to retrieve info from
 * @param vmi_p Pointer to video mode info
 * @return int 
 */
int my_vbe_get_mode_info(uint16_t mode, vbe_mode_info_t *vmi_p);

/**
 * @brief Get control info from vbe
 * 
 * @param control_info
 * @return int 
 */
int my_vbe_get_control_info(vg_vbe_contr_info_t* control_info);

/**
 * @brief Get current mode info from vbe
 * 
 * @param video_info Pointer to video mode info that is retrieved
 */
void vg_get_current_video_info(video_info_t* video_info);

/**
 * @brief Get video resolution
 * 
 * @return const vec2du_t* Pointer to video mode's resolution
 */
const vec2du_t* vg_get_resolution();

/**
 * @brief Exits video mode and frees memory used
 * 
 */
void vg_destroy();

/**
 * @brief Draws a pixel with coordinates x and y the wanted color
 * 
 * @param x Pixel's X coordinate
 * @param y Pixel's Y coordinate
 * @param color Pixel's color
 */
void vg_draw_pixel(uint16_t x, uint16_t y, uint32_t color);

/**
 * @brief Set whole frame buffer to zero
 * 
 */
void vg_clear_screen();

/**
 * @brief Copy frame buffer to GPU ram address
 * (Enables double buffering)
 */
void vg_copy_buffer();

/**
 * @brief Load bitmap into memory
 * 
 * @param xpm xpm file to be loaded
 * @param img Pointer to be filled with xpm data
 * @return int int 0 if succeded, -1 if invalid xpm
 */
int vg_load_xpm(xpm_map_t xpm, xpm_image_t* img);

/**
 * @brief Copy xpm into memory
 * 
 * @param img xmp to copy into x/y GPU buffer location
 * @param x Screen horizontal location
 * @param y Screen vertical location
 * @return int 0 if succeded, -1 if invalid xpm
 */
int vg_draw_xpm(xpm_image_t* img, uint16_t x, uint16_t y);

/**
 * @brief Sets frame area taken by xpm to zero
 * 
 * @param img xpm to clear from screen
 * @param x Screen horizontal location
 * @param y Screen vertical location
 * @return int 0 if succeded, -1 if invalid xpm
 */
int vg_clear_xpm(xpm_image_t* img, uint16_t x, uint16_t y);

/**
 * @brief Load bitmap into memory
 * 
 * @param filename Bitmap file location
 * @return bitmap_t* Pointer to be filled with bitmap data
 */
bitmap_t* vg_load_bitmap(const char* filename);

/**
 * @brief Copy whole bitmap pixel by pixel into x/y GPU buffer location
 * (Allows transparency throught color mask)
 * 
 * @param bitmap Bitmap to copy
 * @param x Screen horizontal location
 * @param y Screen vertical location
 */
void vg_draw_bitmap(bitmap_t* bitmap, int x, int y);

/**
 * @brief Copy whole bitmap line by line into x/y GPU buffer location
 * (Doesn't allow transparency)
 * 
 * @param bitmap Bitmap to copy
 * @param x Screen horizontal location
 * @param y Screen vertical location
 */
void vg_draw_bitmap_by_line(bitmap_t* bitmap, int x, int y);

/**
 * @brief Copy area defined by pos and size into x/y GPU buffer location
 * 
 * @param bitmap Bitmap to copy
 * @param x Screen horizontal location
 * @param y Screen vertical location
 * @param pos Top left corner of area from bitmap
 * @param size Width/Height of area from bitmap
 */
void vg_draw_bitmap_area(bitmap_t* bitmap, int x, int y, vec2du_t* pos, vec2du_t* size);

/**
 * @brief Free memory used by bitmap
 * 
 * @param bitmap Pointer to bitmap
 */
void vg_destroy_bitmap(bitmap_t* bitmap);

#endif /* __VIDEO_H__ */
