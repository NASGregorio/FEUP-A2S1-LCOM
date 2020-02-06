#include <lcom/lcf.h>
#include <math.h>

#include "video.h"
#include "utils.h"

static video_info_t vi;

#pragma pack(1)
typedef struct { 
  char VBESignature [4];
  BCD VBEVersion[2];
  uint32_t OEMStringPtr;
  uint32_t Capabilities;
  uint32_t VideoModePtr;
  uint16_t TotalMemory;
  BCD OEMSoftwareRev [2];
  uint32_t OEMVendorNamePtr;
  uint32_t OEMProductNamePtr;
  uint32_t OEMProductRevPtr;
  uint8_t reserved [222];
  uint8_t OemData [256];
} VBEInfoBlock_t;
#pragma options align = reset


void vbe_to_video_info(vbe_mode_info_t mode_info) {
  vi.x_res = mode_info.XResolution;
  vi.y_res = mode_info.YResolution;
  vi.bits_per_pixel = mode_info.BitsPerPixel;
  vi.color_mode = mode_info.MemoryModel;
  vi.bytes_per_pixel = (int)ceil(vi.bits_per_pixel / 8.0);
  vi.vram_size = vi.x_res * vi.y_res * vi.bytes_per_pixel;

  vi.RedMaskSize = mode_info.RedMaskSize;
  vi.GreenMaskSize = mode_info.GreenMaskSize;
  vi.BlueMaskSize = mode_info.BlueMaskSize;
}

void give_priv_access(phys_bytes base, size_t len) {
  int err = OK;

  struct minix_mem_range mr;
  mr.mr_base = base;
  mr.mr_limit = mr.mr_base + len;
  if ( (err = sys_privctl(SELF, SYS_PRIV_ADD_MEM, &mr)) != OK)
    panic("sys_privctl (ADD_MEM) failed: %d\n", err);
}

int call_sys_int86(reg86_t* r) {
  int err = sys_int86(r);

  if(err != OK ) {
    printf("set_vbe_mode: sys_int86() failed | ret: %d\n", err);
    return SYS_INT86_FAIL;
  }

  if(r->al != 0x4F) {
    printf("VBE function call is not supported\n");
    return VBE_FUNC_NOT_SUPPORTED;
  }

  if(r->ah != 0x00) {

    if(r->ah == VBE_FUNC_CALL_FAIL) {
      printf("VBE Function call failed\n");
      return VBE_FUNC_CALL_FAIL_ERROR;
    } 

    else if(r->ah == VBE_FUNC_NOT_SUP_IN_HW) {
      printf("VBE Function is not supported in current HW configuration\n");
      return VBE_FUNC_NOT_SUP_IN_HW_ERROR;
    }

    else if(r->ah == VBE_FUNC_INVAL_IN_CURR_MODE) {
      printf("VBE Function is invalid in current video mode\n");
      return VBE_FUNC_INVAL_IN_CURR_MODE_ERROR;
    }

    else {
      printf("VBE Unknown error\n");
      return VBE_FUNKNOWN_ERROR;
    }
  }

  return OK;
}

int set_vbe_mode(uint16_t mode) {

  reg86_t r;
  memset(&r, 0, sizeof(reg86_t));

  r.ax = VBE_SET_MODE; // VBE call, function 02 -- set VBE mode
  r.bx = (1<<14) | mode; // set bit 14: linear framebuffer
  r.intno = VBE_VIDEO_INTERRUPT;

  return call_sys_int86(&r);
}

void map_vram(phys_bytes base_ptr, size_t len) {
  give_priv_access(base_ptr, len);

  vi.vram_addr = vm_map_phys(SELF, (void*)base_ptr, len);
  if(vi.vram_addr == MAP_FAILED)
    panic("couldn't map video memory");
}

int my_vbe_get_mode_info(uint16_t mode, vbe_mode_info_t *vmi_p) {
  
  mmap_t buf;
  lm_alloc(sizeof(vbe_mode_info_t),&buf);
  
  reg86_t r;
  memset(&r, 0, sizeof(reg86_t));

  r.ax = VBE_GET_MODE_INFO;
  r.es = PB2BASE(buf.phys);
  r.di = PB2OFF(buf.phys);
  r.cx = mode;
  r.intno = VBE_VIDEO_INTERRUPT;

  //ES:DI    =Pointer to ModeInfoBlock structure
  /*the base address of a segment, a 16 bit-value that should be shifted by 4 to create a 20 bit address, 
  and a 16-bit offset, that should be added to the 20-bit segment address.*/

  int err = call_sys_int86(&r);

  if(err != OK) {
    lm_free(&buf);
    return err;
  }

  memcpy(vmi_p, buf.virt, sizeof(vbe_mode_info_t));
  lm_free(&buf);
  return OK;
}

int my_vbe_get_control_info(vg_vbe_contr_info_t* control_info) {

  mmap_t buf;

  reg86_t r;
  memset(&r, 0, sizeof(reg86_t));

  VBEInfoBlock_t block;

  lm_alloc(sizeof(VBEInfoBlock_t), &buf);

  memcpy(buf.virt, "VBE2", 4);
  vir_bytes base = (vir_bytes)buf.virt - buf.phys;

  r.ax = 0x4F00;
  r.es = PB2BASE(buf.phys);
  r.di = PB2OFF(buf.phys);
  r.intno = VBE_VIDEO_INTERRUPT;

  int err = call_sys_int86(&r);

  if(err != OK) {
    lm_free(&buf);
    return err;
  }

  memcpy(&block, buf.virt, buf.size);
  lm_free(&buf);
  
  memcpy(&control_info->VBESignature, &block.VBESignature, sizeof(block.VBESignature));
  memcpy(&control_info->VBEVersion, &block.VBEVersion, sizeof(block.VBEVersion));
  control_info->TotalMemory = block.TotalMemory * 64;

  control_info->OEMString =         (char*) ((uint32_t)base + phys_to_virt(block.OEMStringPtr));
  control_info->VideoModeList =     (void*) ((uint32_t)base + phys_to_virt(block.VideoModePtr));
  control_info->OEMVendorNamePtr =  (char*) ((uint32_t)base + phys_to_virt(block.OEMVendorNamePtr));
  control_info->OEMProductNamePtr = (char*) ((uint32_t)base + phys_to_virt(block.OEMProductNamePtr));
  control_info->OEMProductRevPtr =  (char*) ((uint32_t)base + phys_to_virt(block.OEMProductRevPtr));

  return OK;
}


void* (vg_init)(uint16_t mode) {

  vbe_mode_info_t mode_info;
  if(my_vbe_get_mode_info(mode, &mode_info) != OK)
    return NULL;
  vbe_to_video_info(mode_info);

  printf("%d x %d | Bits: %u | Mode: ", vi.x_res, vi.y_res, vi.bits_per_pixel);
  switch (vi.color_mode)
  {
  case CM_Direct:
    printf("DIRECT\n");
    break;
  case CM_Indexed:
    printf("INDEXED\n");
    break;
  default:
    printf("OTHER\n");
    break;
  }

  map_vram(mode_info.PhysBasePtr, vi.vram_size);

  int err = OK;
  err = set_vbe_mode(mode);

  if(err != OK)
    return NULL;

  return vi.vram_addr;
  return OK;
}

int (vg_draw_rectangle)(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color) {

  int err = OK;

  for (size_t i = y; i < y + height; i++)
  {
    err = vg_draw_hline(x, i, width, color);
      if(err != OK)
        return err;
  }

  return OK;
}

int (vg_draw_hline)(uint16_t x, uint16_t y, uint16_t len, uint32_t color) {

  for (uint16_t i = x; i < x+len; i++) {
    vg_draw_pixel(i, y, color);
  }

  return OK;
}

int vg_load_xpm(xpm_map_t xpm, xpm_image_t* img) {
  enum xpm_image_type type = 0;
  printf("bits_per_pixel: %u\n", vi.bits_per_pixel);

  switch (vi.bits_per_pixel)
  {
    case 8:
      type = XPM_INDEXED;
      break;
    case 15:
      type = XPM_1_5_5_5;
      break;
    case 24:
      type = XPM_8_8_8;
      break;
    case 16:
      type = XPM_5_6_5;
      break;
    case 32:
      type = XPM_8_8_8_8;
      break;
    default:
      type = INVALID_XPM;
      break;
  }

  uint8_t* sprite = xpm_load(xpm, type, img);

  if(sprite == NULL) {
    printf("Failed to load xpm\n");
    return VG_XPM_LOAD_FAIL;
  }

  return OK;
}

int vg_draw_xpm(xpm_image_t* img, uint16_t x, uint16_t y) {

  if(img == NULL) {
    printf("Invalid XPM image\n");
    return -1;
  }

  for (uint16_t j = 0; j < img->height; j++) {
    for (uint16_t i = 0; i < img->width; i++) {
      vg_draw_pixel(x+i, y+j, img->bytes[j*img->width+i]);
    }
  }

  return OK;
}

int vg_clear_xpm(xpm_image_t* img, uint16_t x, uint16_t y) {

  if(img == NULL) {
    printf("Invalid XPM image\n");
    return -1;
  }

  for (uint16_t j = 0; j < img->height; j++) {
    for (uint16_t i = 0; i < img->width; i++) {
      vg_draw_pixel(x+i, y+j, 0);
    }
  }
  
  return OK;
}

void vg_clear_screen() {
  memset(vi.vram_addr, 0, vi.vram_size);
}

void vg_draw_pixel(uint16_t x, uint16_t y, uint32_t color) {

  if ( x < 0 || x >= vi.x_res || y < 0 || y >= vi.y_res)
    return;

  memcpy(vi.vram_addr + (x + y * vi.x_res) * vi.bytes_per_pixel, &color, vi.bytes_per_pixel);	
}

void vg_get_current_video_info(video_info_t* video_info) {
  *video_info = vi;
}
