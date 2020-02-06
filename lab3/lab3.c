#include <lcom/lcf.h>

#include <lcom/lab3.h>
#include <lcom/timer.h>

#include <stdbool.h>
#include <stdint.h>

#include "keyboard.h"
#include "kbc.h"
#include "i8042.h"

#ifdef LAB3
extern int sys_inb_counter;
#endif

extern int irq_counter;

// uint32_t stat = 0;	

#define CODE_IS_2_BYTE(x) ((x) == SCANCODE_2_BYTE)

int main(int argc, char *argv[]) {
  // sets the language of LCF messages (can be either EN-US or PT-PT)
  lcf_set_language("EN-US");

  // enables to log function invocations that are being "wrapped" by LCF
  // [comment this out if you don't want/need it]
  lcf_trace_calls("/home/lcom/labs/lab3/trace.txt");

  // enables to save the output of printf function calls on a file
  // [comment this out if you don't want/need it]
  lcf_log_output("/home/lcom/labs/lab3/output.txt");

  // handles control over to LCF
  // [LCF handles command line arguments and invokes the right function]
  if (lcf_start(argc, argv))
    return 1;

  // LCF clean up tasks
  // [must be the last statement before return]
  lcf_cleanup();

  return 0;
}



int(kbd_test_scan)() {

  uint8_t irq_set; // Number of the bit used to subscribe to interrupt

  int err = OK;

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
            if(get_scancode(&code) == OK)
              kbd_print_scancode(code.make,code.size,code.bytes); //  print scancode

            // look for ESC break code
            if(code.bytes[0] == BREAKCODE_ESC) {
              //  exit
              STOP = 1;
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

  #ifdef LAB3
  // print number of kernel calls
  err = kbd_print_no_sysinb(sys_inb_counter);
  #endif

  err = kb_unsubscribe_int();


  return err;
}

int(kbd_test_poll)() {

  Scancode_t code;

  int err = OK;

  bool got_esc_break = false;

  // Poll until ESC break code (0x81)
  while(!got_esc_break) {

    kbc_ph();

    //  assemble_scancode
    if(get_scancode(&code) == OK)
      kbd_print_scancode(code.make,code.size,code.bytes); //  print scancode

    // look for ESC break code
    if(code.bytes[0] == BREAKCODE_ESC) {
      got_esc_break = true;
    }
  }

  uint8_t data;

  kbc_write_cmd(0x20);

  kbc_read_from_reg(KBC_OUT_BUF, &data);

  kbc_write_cmd_w_arg(0x60, (data | BIT(0)));

  #ifdef LAB3
  // print number of kernel calls
  err = kbd_print_no_sysinb(sys_inb_counter);
  #endif

  return err;
}

int(kbd_test_timed_scan)(uint8_t n) {

  uint8_t irq_set_timer; // Number of the bit used to subscribe to interrupt
  uint8_t irq_set_kb; // Number of the bit used to subscribe to interrupt

  int err = OK;

  err = kb_subscribe_int(&irq_set_kb);
  if(err != OK)
    return err;

  err = timer_subscribe_int(&irq_set_timer);
  if(err != OK)
    return err;

  int ipc_status;
  message msg;
  int r;

  Scancode_t code;

  int STOP = 0;
  int irq_counter = 60 * n;

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
          if (msg.m_notify.interrupts & BIT(irq_set_timer)) {     // Check if subscribed to this interrupt
              timer_int_handler();                            // Call interrupt handler
              if(irq_counter % 60 == 0)                             // Every second, print elapsed time
                printf("%d\n", irq_counter/60);
                //timer_print_elapsed_time();
              if(irq_counter <= 0)
              {
                STOP = 1;
                break;
              }
              irq_counter--;
          }
          if (msg.m_notify.interrupts & BIT(irq_set_kb)) {     // Check if subscribed to this interrupt

            irq_counter = 60 * n;

            kbc_ih();                                       // Call interrupt handler

            //  assemble_scancode
            if(get_scancode(&code) == OK)
              kbd_print_scancode(code.make,code.size,code.bytes); //  print scancode

            // look for ESC break code
            if(code.bytes[0] == BREAKCODE_ESC) {
              //  exit
              STOP = 1;
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

  #ifdef LAB3
  // print number of kernel calls
  err = kbd_print_no_sysinb(sys_inb_counter);
  #endif

  err = kb_unsubscribe_int();


  return err;
}
