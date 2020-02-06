#include <lcom/lcf.h>
#include <lcom/lab2.h>

#include <stdbool.h>
#include <stdint.h>
extern int irq_counter;


int main(int argc, char *argv[]) {
  // sets the language of LCF messages (can be either EN-US or PT-PT)
  lcf_set_language("EN-US");

  // enables to log function invocations that are being "wrapped" by LCF
  // [comment this out if you don't want/need it]
  lcf_trace_calls("/home/lcom/labs/lab2/trace.txt");

  // enables to save the output of printf function calls on a file
  // [comment this out if you don't want/need it]
  lcf_log_output("/home/lcom/labs/lab2/output.txt");

  // handles control over to LCF
  // [LCF handles command line arguments and invokes the right function]
  if (lcf_start(argc, argv))
    return 1;

  // LCF clean up tasks
  // [must be the last statement before return]
  lcf_cleanup();

  return 0;
}

int(timer_test_read_config)(uint8_t timer, enum timer_status_field field) {

  uint8_t st;

  if(timer_get_conf(timer, &st) != 0) {
    fprintf(stderr, "Failed to get timer config\n");
    return 1;
  }

  if(timer_display_conf(timer, st, field) != 0) {
    fprintf(stderr, "Failed to display timer config\n");
    return 1;
  }

  return 0;
}

int(timer_test_time_base)(uint8_t timer, uint32_t freq) {

  if(timer_set_frequency(timer, freq)) {
    fprintf(stderr, "Failed to set timer frequency\n");
    return 1;
  }

  return 0;
}

int(timer_test_int)(uint8_t time) {

  
  if(timer_set_frequency(0, 60) != 0) {
    fprintf(stderr, "Failed to set timer frequency to 60Hz\n");
    return 1;
  }

  uint8_t irq_set; // Number of the bit used to subscribe to interrupt

  if(timer_subscribe_int(&irq_set) != 0) {
    fprintf(stderr, "Failed to subscribe to interrupt notifications\n");
    return 1;
  }

  int ipc_status;
  message msg;
  int r;
 
  while( irq_counter < time*60 ) {

    // Get a request message
    if ( (r = driver_receive(ANY, &msg, &ipc_status)) != 0 ) { 
      printf("driver_receive failed with: %d", r);
      continue;
    }

    // Check if message is notification
    if (is_ipc_notify(ipc_status)) {
      switch (_ENDPOINT_P(msg.m_source)) {
        case HARDWARE:                                      // Hardware interrupt notification
          if (msg.m_notify.interrupts & BIT(irq_set)) {     // Check if subscribed to this interrupt
            timer_int_handler();                            // Call interrupt handler
            if(irq_counter % 60 == 0)                             // Every second, print elapsed time
              timer_print_elapsed_time();
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

  if(timer_unsubscribe_int() != 0) {
    fprintf(stderr, "Failed to unsubsribe to interrupt notifications\n");
    return 1;
  }

  return 0;
}
