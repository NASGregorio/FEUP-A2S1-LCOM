#include <lcom/lcf.h>
#include <fcntl.h>
#include <math.h>

#include "video.h"
#include "utils.h"

#define HEADER_SIZE 14

static video_info_t vi;

static uint8_t* frame_buffer;

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
  vi.res.x = mode_info.XResolution;
  vi.res.y = mode_info.YResolution;
  vi.bits_per_pixel = mode_info.BitsPerPixel;
  vi.color_mode = mode_info.MemoryModel;
  vi.bytes_per_pixel = (int)ceil(vi.bits_per_pixel / 8.0);
  vi.vram_size = vi.res.x * vi.res.y * vi.bytes_per_pixel;

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

void vg_get_current_video_info(video_info_t* video_info) {
  *video_info = vi;
}

const vec2du_t* vg_get_resolution() {
  return &vi.res;
}



void* (vg_init)(uint16_t mode) {

  vbe_mode_info_t mode_info;
  if(my_vbe_get_mode_info(mode, &mode_info) != OK)
    return NULL;
  vbe_to_video_info(mode_info);

  printf("%d x %d | Bits: %u | Mode: ", vi.res.x, vi.res.y, vi.bits_per_pixel);
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

  frame_buffer = malloc(vi.vram_size * sizeof(uint8_t));

  return vi.vram_addr;
}

void vg_destroy() {
  vg_exit();

  free(frame_buffer);
}



int (vg_draw_rectangle)(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color) {

  int err = OK;

  uint16_t len_x = MIN(vi.res.x, width);
  uint16_t len_y = MIN(vi.res.y, y + height);


  for (size_t i = y; i < len_y; i++)
  {
    err = vg_draw_hline(x, i, len_x, color);
      if(err != OK)
        return err;
  }

  return OK;
}

int (vg_draw_hline)(uint16_t x, uint16_t y, uint16_t len, uint32_t color) {

  //uint16_t max = MIN(x+len, MAX_16_BIT);
  for (uint16_t i = 0; i < len; i++) {
    vg_draw_pixel(x+i, y, color);
  }

  return OK;
}

void vg_draw_pixel(uint16_t x, uint16_t y, uint32_t color) {

  if ( x >= vi.res.x || y >= vi.res.y)
    return;

  memcpy(frame_buffer + (x + y * vi.res.x) * vi.bytes_per_pixel, &color, vi.bytes_per_pixel);	
}

void vg_draw_pixel_ptr(uint16_t x, uint16_t y, uint16_t* color) {

  if ( x >= vi.res.x || y >= vi.res.y)
    return;

  memcpy(frame_buffer + (x + y * vi.res.x) * vi.bytes_per_pixel, color, vi.bytes_per_pixel);	
}

void vg_clear_screen() {
  memset(frame_buffer, 0, vi.vram_size);
}

void vg_copy_buffer() {
	memcpy(vi.vram_addr, frame_buffer, vi.vram_size);
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




bitmap_t* vg_load_bitmap(const char* path) {

  // Open file
  int fd = open(path, O_RDONLY);
  if (fd < 0)
    return NULL;

  bitmap_file_header_t file_header;

  // Check bitmap signature
  pread(fd, &file_header, 2, 0);
  if(file_header.type != 0x4D42) {
    close(fd);
    return NULL;
  }

  // Read file header
  pread(fd, &file_header, HEADER_SIZE, 0);

  // Read info header
  bitmap_info_header_t* info = malloc(sizeof(bitmap_info_header_t));
  pread(fd, info, 4, HEADER_SIZE);
  pread(fd, info, info->bV5Size, HEADER_SIZE);

  // Read bitmap data
  uint8_t* data = (uint8_t*)malloc(info->bV5SizeImage);
  pread(fd, data, info->bV5SizeImage, file_header.offset);
  close(fd);

  // Prepare bitmap
  bitmap_t* bitmap = malloc(sizeof(bitmap_t));
  bitmap->info = info;
  bitmap->data = data;

  printf("ALPHA:     %u\n", info->bV5AlphaMask);



  // uint8_t* ptr = (uint8_t*)&file_header;
  // printf("FILE HEADER\n");
  // for (size_t i = 0; i < HEADER_SIZE; i++)
  // {
  //   if(i != 0 && i%16 == 0)
  //     printf("\n");
  //   printf("%02x ", (ptr)[i]);
  // }
  // printf("\n\n");
  // printf("TYPE:     %u\n", file_header.type);
  // printf("SIZE:     %u\n", file_header.size);
  // printf("RESERVED: %u\n", file_header.reserved);
  // printf("OFFSET:   %u\n", file_header.offset);
    
  // ptr = (uint8_t*)&info_header;
  // printf("INFO HEADER\n");
  // for (size_t i = 14; i < info->bV5Size+14; i++)
  // {
  //   if(i != 0 && i%16 == 0)
  //     printf("\n");
  //   printf("%02x ", (ptr)[i-14]);
  // }
  // printf("\n\n");
  // printf("SIZE:        %u\n", info->bV5Size);
  // printf("WIDTH:       %u\n", info->bV5Width);
  // printf("HEIGHT:      %u\n", info->bV5Height);
  // printf("PLANES:      %u\n", info->bV5Planes);
  // printf("BITCOUNT:    %u\n", info->bV5BitCount);
  // printf("COMPRESSION: %u\n", info->bV5Compression);
  // printf("IMG SIZE:    %u\n", info->bV5SizeImage);

  // printf("BITMAP DATA\n");
  // for (size_t i = 0; i < info->bV5SizeImage; i++)
  // {
  //   if(i != 0 && i%16 == 0)
  //     printf("\n");
  //   printf("%02x ", data[i]);
  // }
  // printf("\n\n");

  return bitmap;
}

void vg_draw_bitmap_by_line(bitmap_t* bitmap, int x, int y) {

  // bitmap space
  int bx = 0;
  int bw = bitmap->info->bV5Width;
  int bh = bitmap->info->bV5Height;

  // screen space
  int sx = x;

  // Check out of screen bitmap
  if (x < -bw || x > vi.res.x || y < -bh || y > vi.res.y)
    return;

  // If part outside left side screen
  if(sx < 0) {
    bx = bx - sx;                 // copy only visible part of bitmap (left side)
    bw = MIN(vi.res.x, bw - bx);  // copy only visible part of bitmap (right side)
    sx = 0;                       // no need to draw left part outside screen
  }

  // If part outside right side screen
  else if (sx + bw > vi.res.x) {
    bw = vi.res.x - sx;
  }

  int sy = 0;
  // for (int i = bh - 1; i >= 0; i--) {
  //   sy = y - i + bh;
  for (int i = 0; i < bh; i++) {
    sy = y + bh - 1 - i;

    if (sy < 0 || sy >= vi.res.y)
      continue;

    memcpy( frame_buffer + (sx + sy * vi.res.x) * vi.bytes_per_pixel, 
            bitmap->data + (bx + i * bitmap->info->bV5Width) * vi.bytes_per_pixel, 
            bw * vi.bytes_per_pixel);
  }
}

void vg_draw_bitmap(bitmap_t* bitmap, int x, int y) {

  if(bitmap == NULL) {
    printf("Invalid BMP image\n");
    return;
  }

  int bw = bitmap->info->bV5Width;
  int bh = bitmap->info->bV5Height;
  uint16_t* color;

  for (int j = 1; j < bh; j++)
  {
    for (int i = 0; i < bw; i++)
    {
      if(x + i < 0 || x + i >= vi.res.x || y + j < 0 || y + j >= vi.res.y)
        continue;

      color = (uint16_t*) (bitmap->data + (i + (bh-j) * bitmap->info->bV5Width) * vi.bytes_per_pixel);

      if(*color == 0xF81F)
        continue;
      
      vg_draw_pixel_ptr(x+i, y+j, color);
    }
  }
}

void vg_draw_bitmap_area(bitmap_t* bitmap, int x, int y, vec2du_t* pos, vec2du_t* size) {

  if(bitmap == NULL) {
    printf("Invalid BMP image\n");
    return;
  }

  int bh = bitmap->info->bV5Height;

  int bx = (pos->x);
  for (int j = 0; j < size->y; j++)
  {
    int by = ((bh-j-pos->y)) * bitmap->info->bV5Width;

    memcpy( frame_buffer + (x + (y+j) * vi.res.x) * vi.bytes_per_pixel, 
            bitmap->data + (bx + by) * vi.bytes_per_pixel, 
            size->x * vi.bytes_per_pixel);
  }
}

void vg_destroy_bitmap(bitmap_t* bitmap) {
  if (bitmap == NULL)
      return;

  free(bitmap->info);
  free(bitmap->data);
  free(bitmap);
}
