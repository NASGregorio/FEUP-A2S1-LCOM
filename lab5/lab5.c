// IMPORTANT: you must include the following line in all your C files
#include <lcom/lcf.h>

#include <lcom/lab5.h>

#include <stdint.h>
#include <stdio.h>
#include <math.h>

// Any header files included below this line should have been created by you

#include "video.h"
#include "keyboard.h"


int main(int argc, char *argv[]) {
  // sets the language of LCF messages (can be either EN-US or PT-PT)
  lcf_set_language("EN-US");

  // enables to log function invocations that are being "wrapped" by LCF
  // [comment this out if you don't want/need it]
  lcf_trace_calls("/home/lcom/labs/lab5/trace.txt");

  // enables to save the output of printf function calls on a file
  // [comment this out if you don't want/need it]
  lcf_log_output("/home/lcom/labs/lab5/output.txt");

  // handles control over to LCF
  // [LCF handles command line arguments and invokes the right function]
  if (lcf_start(argc, argv))
    return 1;

  // LCF clean up tasks
  // [must be the last statement before return]
  lcf_cleanup();

  return 0;
}

int do_until_esc() {

  int err = OK;
  uint8_t irq_set; // Number of the bit used to subscribe to interrupt

  err = kb_subscribe_int(&irq_set);
  if(err != OK)
    return err;

  int ipc_status;
  message msg;
  int r;
  Scancode_t code;
  int STOP = 0;

  // Until ESC is released
  while( !STOP ) {

    // Get a request message
    if ( (r = driver_receive(ANY, &msg, &ipc_status)) != 0 ) {
      printf("driver_receive failed with: %d", r);
      continue;
    }

    if (is_ipc_notify(ipc_status)) {                        // Check if message is notification
      switch (_ENDPOINT_P(msg.m_source)) {
        case HARDWARE:                                      // Hardware interrupt notification
          if (msg.m_notify.interrupts & BIT(irq_set)) {     // Check if subscribed to this interrupt

            kbc_ih();                                       // Call interrupt handler

            //  assemble_scancode
            if(get_scancode(&code) == OK) {
              // look for ESC break code
              if(code.bytes[0] == BREAKCODE_ESC) {
                //  exit
                STOP = 1;
              }
            }
          }
          break;
        default:
          // no other notifications expected: do nothing
          break;
      }
    }
    else {
      // No standard messages expected: do nothing
    }
  }

  err = kb_unsubscribe_int();

  return err;
}

int(video_test_init)(uint16_t mode, uint8_t delay) {

  uint8_t* vram_addr = vg_init(mode);
  if(vram_addr == NULL)
    return VG_INIT_ERROR;

  tickdelay(micros_to_ticks(delay*1000000));

  return vg_exit();
}

int(video_test_rectangle)(uint16_t mode, uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color) {

  uint8_t* vram_addr = vg_init(mode);
  if(vram_addr == NULL)
    return VG_INIT_ERROR;
  
  vg_draw_rectangle(x, y, width, height, color);

  do_until_esc();

  int err = vg_exit();

  return err;
}

int(video_test_pattern)(uint16_t mode, uint8_t no_rectangles, uint32_t first, uint8_t step) {

  uint8_t* vram_addr = vg_init(mode);
  if(vram_addr == NULL)
    return VG_INIT_ERROR;

  if(no_rectangles > 0) {
    
    video_info_t vi;
    vg_get_current_video_info(&vi);

    uint16_t width = vi.x_res / (uint16_t)no_rectangles;
    uint16_t height = vi.y_res / (uint16_t)no_rectangles;

    color_t first_color;
    color_t step_color;
    first_color.color = step_color.color = first;

    for (uint8_t j = 0; j < no_rectangles; j++)
    {
      for (uint8_t i = 0; i < no_rectangles; i++)
      {
        if(vi.color_mode == CM_Indexed) {
          step_color.color = (first_color.color + (j * no_rectangles + i) * step) % (1 << vi.bits_per_pixel);
        }
        else {
          step_color.colors[2] = (first_color.colors[2] + i * step) % (1 << vi.RedMaskSize);
          step_color.colors[1] = (first_color.colors[1] + j * step) % (1 << vi.GreenMaskSize);
          step_color.colors[0] = (first_color.colors[0] + (i + j) * step) % (1 << vi.BlueMaskSize);
        }
        vg_draw_rectangle(width*i, height*j, width, height, step_color.color);
      }
    }
  }


  do_until_esc();

  int err = vg_exit();

  return err;
}

int(video_test_xpm)(xpm_map_t xpm, uint16_t x, uint16_t y) {
  
  uint8_t* vram_addr = vg_init(0x105);
  if(vram_addr == NULL)
    return VG_INIT_ERROR;

  xpm_image_t img;
  vg_load_xpm(xpm, &img);
  vg_draw_xpm(&img, x, y);

  do_until_esc();

  return vg_exit();
}

int(video_test_move)(xpm_map_t xpm, uint16_t xi, uint16_t yi, uint16_t xf, uint16_t yf, int16_t speed, uint8_t fr_rate) {

  uint8_t irq_set_timer; // Number of the bit used to subscribe to interrupt
  uint8_t irq_set_kb; // Number of the bit used to subscribe to interrupt

  int err = OK;

  err = kb_subscribe_int(&irq_set_kb);
  if(err != OK)
    return err;

  err = timer_subscribe_int(&irq_set_timer);
  if(err != OK)
    return err;

  uint8_t* vram_addr = vg_init(0x105);
  if(vram_addr == NULL)
    return VG_INIT_ERROR;

  int ipc_status;
  message msg;
  int r;
  Scancode_t code;
  int STOP = 0;

  xpm_image_t img;
  vg_load_xpm(xpm, &img);

  int ipf = sys_hz() / fr_rate; // interrupts per frame
  int irq_counter = 0;
  int frame_counter = 0;
  int moving = 1;


  int dir = 1;
  uint16_t dx = abs(xf - xi);
  uint16_t dy = abs(yf - yi);
  uint16_t abs_speed = abs(speed);
  uint16_t curr = 0;
  uint16_t currX = 0;
  uint16_t currY = 0;
  uint16_t limit = 0;

  if(dx > 0) {
    curr = xi;
    currX = xi;
    currY = yi;
    if(xf > xi ) {
      dir = 1;
      limit = (speed > 0) ? xi + (dx - (dx % abs_speed)) : xf;
    }
    else {
      dir = -1;
      limit = (speed > 0) ? xf + (dx % abs_speed) : xf;
    }
  }
  else {
    curr = yi;
    currX = xi;
    currY = yi;
    if(yf > yi ) {
      dir = 1;
      limit = (speed > 0) ? yi + (dy - (dy % abs_speed)) : yf;
    }
    else {
      dir = -1;
      limit = (speed > 0) ? yf + (dy % abs_speed) : yf;
    }
  }


  printf("dir: %d\n", dir);
  printf("dx: %u\n", dx);
  printf("dy: %u\n", dy);
  printf("speed: %d\n", speed);
  printf("abs_speed: %d\n", abs_speed);
  printf("curr %u\n", curr);
  printf("limit: %u\n", limit);


  // Draw first position
  vg_draw_xpm(&img, currX, currY);


  // Until ESC is released
  while( !STOP ) {
    // Get a request message
    if ( (r = driver_receive(ANY, &msg, &ipc_status)) != 0 ) {
      printf("driver_receive failed with: %d", r);
      continue;
    }

    if (is_ipc_notify(ipc_status)) {
      switch (_ENDPOINT_P(msg.m_source)) {
        case HARDWARE:

          if (msg.m_notify.interrupts & BIT(irq_set_kb)) {
            kbc_ih();
            if(get_scancode(&code) == OK)
              if(code.bytes[0] == BREAKCODE_ESC)
                STOP = 1;
          }

          if(moving == 0) {
            continue;
          }

          if (msg.m_notify.interrupts & BIT(irq_set_timer)) {

            if((++irq_counter) % ipf == 0) {


              if(speed > 0) {
                //vg_clear_screen();
                vg_clear_xpm(&img, currX, currY);
                curr += (abs_speed * dir);
                if(dx > 0)
                  currX = curr;
                else
                  currY = curr;
                if((curr*dir) >= limit) {
                  currX = xf;
                  currY = yf;
                  moving = false;
                }
                vg_draw_xpm(&img, currX, currY);
                continue;
              }
              if((++frame_counter) >= abs_speed) {
                frame_counter = 0;
                //vg_clear_screen();
                vg_clear_xpm(&img, currX, currY);
                curr += (dir);
                if(dx > 0)
                  currX = curr;
                else
                  currY = curr;
                if((curr*dir) >= limit) {
                  currX = xf;
                  currY = yf;
                  moving = false;
                }
                vg_draw_xpm(&img, currX, currY);
                continue;
              }
            }
          }
          break;
        default:
          // no other notifications expected: do nothing
          break;
      }
    }
    else {
      // No standard messages expected: do nothing
    }
  }

  timer_unsubscribe_int();
  kb_unsubscribe_int();
  
  return vg_exit();
}

int(video_test_controller)() {

  vg_vbe_contr_info_t ci;
  my_vbe_get_control_info(&ci);
  vg_display_vbe_contr_info(&ci);        

  return OK;
}
