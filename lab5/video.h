#ifndef __VIDEO_H__
#define __VIDEO_H__

#include <stdint.h>

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

typedef union {
  uint32_t color;
  uint8_t colors[4];
} color_t;

typedef enum {
  CM_Indexed = 4,
  CM_Direct = 6,
  CM_Other = 0
} color_mode_t;

typedef struct {
  uint8_t* vram_addr;
  size_t vram_size;
  uint16_t x_res;
  uint16_t y_res;
  uint8_t bits_per_pixel;
  uint8_t bytes_per_pixel;
  color_mode_t color_mode;
  uint8_t RedMaskSize;        
  uint8_t GreenMaskSize;
  uint8_t BlueMaskSize;
} video_info_t;

void vg_draw_pixel(uint16_t x, uint16_t y, uint32_t color);

int vg_load_xpm(xpm_map_t xpm, xpm_image_t* img);

int vg_draw_xpm(xpm_image_t* img, uint16_t x, uint16_t y);

int vg_clear_xpm(xpm_image_t* img, uint16_t x, uint16_t y);

void vg_clear_screen();

void vg_get_current_video_info(video_info_t* video_info);


int my_vbe_get_mode_info(uint16_t mode, vbe_mode_info_t *vmi_p);

int my_vbe_get_control_info(vg_vbe_contr_info_t* control_info);

#endif /* __VIDEO_H__ */
