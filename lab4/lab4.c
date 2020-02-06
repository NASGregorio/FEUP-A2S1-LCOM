// IMPORTANT: you must include the following line in all your C files
#include <lcom/lcf.h>

#include <stdint.h>
#include <stdio.h>

// Any header files included below this line should have been created by you

#include "mouse.h"
#include "kbc.h"
#include "i8042.h"
#include "gesture_logic_and.h"

int main(int argc, char *argv[]) {
  // sets the language of LCF messages (can be either EN-US or PT-PT)
  lcf_set_language("EN-US");

  // enables to log function invocations that are being "wrapped" by LCF
  // [comment this out if you don't want/need/ it]
  lcf_trace_calls("/home/lcom/labs/lab4/trace.txt");

  // enables to save the output of printf function calls on a file
  // [comment this out if you don't want/need it]
  lcf_log_output("/home/lcom/labs/lab4/output.txt");

  // handles control over to LCF
  // [LCF handles command line arguments and invokes the right function]
  if (lcf_start(argc, argv))
    return 1;

  // LCF clean up tasks
  // [must be the last statement before return]
  lcf_cleanup();

  return 0;
}


int (mouse_test_packet)(uint32_t cnt) {

  uint8_t irq_set; // Number of the bit used to subscribe to interrupt
  int err = OK;
  uint8_t ack_reply;

  //set stream mode
  if( (err = send_mouse_cmd(&ack_reply, MS_SET_STREAM_MODE)) != OK) {
    printf("Failed to set stream mode | ret: %d\n", err);
    return err;
  }

  //enable data reporting
  if( (err = send_mouse_cmd(&ack_reply, MS_ENABLE_DATA_REPORT)) != OK) {
    printf("Failed to enable data reporting | ret: %d\n", err);
    return err;
  }

  err = mouse_subscribe_int(&irq_set);
  if(err != OK)
    return err;

  mouse_packet_t my_mouse_packet;
  struct packet mouse_packet;
  size_t packet_counter = 0;

  int ipc_status;
  message msg;
  int r;

  while( packet_counter < cnt ) {

    // Get a request message
    if ( (r = driver_receive(ANY, &msg, &ipc_status)) != 0 ) {
      printf("driver_receive failed with: %d", r);
      continue;
    }

    if (is_ipc_notify(ipc_status)) {                        // Check if message is notification
      switch (_ENDPOINT_P(msg.m_source)) {
        case HARDWARE:                                      // Hardware interrupt notification
          if (msg.m_notify.interrupts & BIT(irq_set)) {     // Check if subscribed to mouse

            mouse_ih();                                     // Call interrupt handler

            // assemble mouse packet
            if(get_mouse_packet(&my_mouse_packet) == OK)
            {
              memcpy(&mouse_packet, &my_mouse_packet, sizeof(mouse_packet));
              mouse_print_packet(&mouse_packet); //  print mouse packet
              packet_counter++;
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

  err = mouse_unsubscribe_int();
  if(err != OK)
    return err;

  if( (err = send_mouse_cmd(&ack_reply, MS_DISABLE_DATA_REPORT)) != OK) {
    printf("Failed to disable data reporting | ret: %d\n", err);
    return err;
  }

  return err;
}

int (mouse_test_remote)(uint16_t period, uint8_t cnt) {

  uint8_t ack_reply;
  int err = OK;

  //disable data reporting
  if( (err = send_mouse_cmd(&ack_reply, MS_DISABLE_DATA_REPORT)) != OK) {
    printf("Failed to disable data reporting | ret: %d\n", err);
    return err;
  }

  //set remote mode
  if( (err = send_mouse_cmd(&ack_reply, MS_SET_REMOTE_MODE)) != OK) {
    printf("Failed to set remote mode | ret: %d\n", err);
    return err;
  }

  //disable interrupts
  uint8_t data;
  kbc_write_to_reg(KBC_CMD_REG, KBC_ASK_CMDB);
  if( (err = util_sys_inb(KBC_OUT_BUF, &data)) != OK)
    return KBC_OUTBUF_ERR;
  //kbc_read_from_reg(KBC_OUT_BUF, &data);
  kbc_write_cmd_w_arg(KBC_WRT_CMDB, (data & ~KBC_INT_MS));

  mouse_packet_t my_mouse_packet;
  struct packet mouse_packet;
  size_t packet_counter = 0;

  while( packet_counter < cnt ) {

    uint8_t ack_reply;

    //read data
    if( (err = send_mouse_cmd(&ack_reply, MS_READ_DATA)) != OK) {
      printf("Failed to read data | ret: %d\n", err);
      continue;
    }
    
    while ( 1 ) {
      mouse_ph();

      // assemble mouse packet
      if(get_mouse_packet(&my_mouse_packet) == OK)
      {
        memcpy(&mouse_packet, &my_mouse_packet, sizeof(mouse_packet));
        mouse_print_packet(&mouse_packet); //  print mouse packet
        packet_counter++;
        break;
      }
    }
    tickdelay(micros_to_ticks(period * 1000));
  }

  //set stream mode
  if( (err = send_mouse_cmd(&ack_reply, MS_SET_STREAM_MODE)) != OK) {
    printf("Failed to set stream mode | ret: %d\n", err);
    return err;
  }

  //disable data report
  if( (err = send_mouse_cmd(&ack_reply, MS_DISABLE_DATA_REPORT)) != OK) {
    printf("Failed to disable data report | ret: %d\n", err);
    return err;
  }

  //restore command byte to default state
  kbc_write_cmd_w_arg(KBC_WRT_CMDB, minix_get_dflt_kbc_cmd_byte());
  if(err != OK)
    return err;

  return OK;
}

int (mouse_test_async)(uint8_t idle_time) {

  uint8_t irq_set_mouse; // Number of the bit used to subscribe to mouse interrupts

  uint8_t irq_set_timer; // Number of the bit used to subscribe to timer interrupts

  int err = OK;
  uint8_t ack_reply;

  //set stream mode
  if( (err = send_mouse_cmd(&ack_reply, MS_SET_STREAM_MODE)) != OK) {
    printf("Failed to set stream mode | ret: %d\n", err);
    return err;
  }

  //enable data report
  if( (err = send_mouse_cmd(&ack_reply, MS_ENABLE_DATA_REPORT)) != OK) {
    printf("Failed to enable data report | ret: %d\n", err);
    return err;
  }

  err = timer_subscribe_int(&irq_set_timer);
  if(err != OK)
    return err;

  err = mouse_subscribe_int(&irq_set_mouse);
  if(err != OK)
    return err;

  mouse_packet_t my_mouse_packet;
  struct packet mouse_packet;
  int irq_total = sys_hz() * idle_time;
  int irq_counter = irq_total;
  int STOP = 0;

  int ipc_status;
  message msg;
  int r;


  while( !STOP ) {

    // Get a request message
    if ( (r = driver_receive(ANY, &msg, &ipc_status)) != 0 ) {
      printf("driver_receive failed with: %d", r);
      continue;
    }

    if (is_ipc_notify(ipc_status)) {                                  // Check if message is notification
      switch (_ENDPOINT_P(msg.m_source)) {
        case HARDWARE:                                                // Hardware interrupt notification
          if (msg.m_notify.interrupts & BIT(irq_set_timer)) {         // Check if subscribed to this interrupt
              timer_int_handler();                                    // Call interrupt handler
              if(irq_counter <= 0)
              {
                STOP = 1;                                             // Break loop if timer reaches 0
                break;
              }
              irq_counter--;

              if(irq_counter*idle_time % irq_total == 0)              // Every second, print elapsed time
                printf("%d\n", (irq_counter*idle_time/irq_total)+1);
          }

          if (msg.m_notify.interrupts & BIT(irq_set_mouse)) {         // Check if subscribed to mouse

            irq_counter = irq_total;                                  // Reset timer

            mouse_ih();                                               // Call interrupt handler

            // assemble mouse packet
            if(get_mouse_packet(&my_mouse_packet) == OK)
            {
              memcpy(&mouse_packet, &my_mouse_packet, sizeof(mouse_packet));
              mouse_print_packet(&mouse_packet);                      //  print mouse packet
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


  err = mouse_unsubscribe_int();
  if(err != OK)
    return err;

  err = timer_unsubscribe_int();
  if(err != OK)
    return err;

  //disable data reporting
  if( (err = send_mouse_cmd(&ack_reply, MS_DISABLE_DATA_REPORT)) != OK) {
    printf("Failed to disable data reporting | ret: %d\n", err);
    return err;
  }

  return err;
}

int (mouse_test_gesture)(uint8_t x_len, uint8_t tolerance) {

  uint8_t irq_set; // Number of the bit used to subscribe to interrupt

  int err = OK;
  uint8_t ack_reply;

  //set stream mode
  if( (err = send_mouse_cmd(&ack_reply, MS_SET_STREAM_MODE)) != OK) {
    printf("Failed to set stream mode | ret: %d\n", err);
    return err;
  }

  //enable data reporting
  if( (err = send_mouse_cmd(&ack_reply, MS_ENABLE_DATA_REPORT)) != OK) {
    printf("Failed to enable data report | ret: %d\n", err);
    return err;
  }

  err = mouse_subscribe_int(&irq_set);
  if(err != OK)
    return err;

  mouse_packet_t current_mouse_packet;
  struct packet mouse_packet;
  gesture_state_t current_state = Gesture_Start;
  int STOP = 0;

  int ipc_status;
  message msg;
  int r;

  while( STOP == 0 ) {

    // Get a request message
    if ( (r = driver_receive(ANY, &msg, &ipc_status)) != 0 ) {
      printf("driver_receive failed with: %d", r);
      continue;
    }

    if (is_ipc_notify(ipc_status)) {                        // Check if message is notification
      switch (_ENDPOINT_P(msg.m_source)) {
        case HARDWARE:                                      // Hardware interrupt notification
          if (msg.m_notify.interrupts & BIT(irq_set)) {     // Check if subscribed to mouse

            mouse_ih();                                     // Call interrupt handler

            // assemble mouse packet
            if(get_mouse_packet(&current_mouse_packet) == OK)
            {
              memcpy(&mouse_packet, &current_mouse_packet, sizeof(mouse_packet));
              mouse_print_packet(&mouse_packet); //  print mouse packet

              if(track_logic_and(&current_state, x_len, tolerance, &current_mouse_packet) == GESTURE_COMPLETE) {
                STOP = 1;
                break;
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

  err = mouse_unsubscribe_int();
  if(err != OK)
    return err;

  //disable data reporting
  if( (err = send_mouse_cmd(&ack_reply, MS_DISABLE_DATA_REPORT)) != OK) {
    printf("Failed to disable data reporting | ret: %d\n", err);
    return err;
  }

  return err;
}
