#include <lcom/lcf.h>

#include "device_manager.h"
#include "kb_manager.h"
#include "mouse_manager.h"
#include "rtc.h"

#include "gs_menu.h"
#include "gs_hold_your_ground.h"
#include "gs_inf_runner.h"
#include "gs_wolfenstein.h"

game_state_t game_state;

static int manager_state_changes() {

  if(game_state.next_state == STATE_CURR)
    return - 1;

  kb_clear_keys_down_up();
  mouse_clear_buttons_down_up();

  if(game_state.exit)
    game_state.exit(&game_state);

  switch (game_state.next_state)
  {
  case STATE_HOLD:
    switch_to_gs_hold(&game_state);
    break;
  case STATE_RUNNER:
    switch_to_gs_runner(&game_state);
    break;
  case STATE_WOLFENSTEIN:
    switch_to_gs_wolfenstein(&game_state);
    break;
  case STATE_MENU:
    switch_to_gs_menu(&game_state);
  default:
    break;
  }

  if(game_state.enter)
    game_state.enter(&game_state);
  game_state.next_state = STATE_CURR;

  return OK;
}

int dm_loop() {

  int ipc_status;
  message msg;
  int r;

  game_state.is_game_running = true;

  while( game_state.is_game_running ) {

    // Get a request message
    if ( (r = driver_receive(ANY, &msg, &ipc_status)) != 0 ) { 
      printf("driver_receive failed with: %d", r);
      continue;
    }

    // Check if message is notification
    if (is_ipc_notify(ipc_status)) {
      switch (_ENDPOINT_P(msg.m_source)) {
        case HARDWARE:

            if (msg.m_notify.interrupts & BIT(game_state.kb_bit)) {

              kb_update_keys();
              //print_kb_state();

              if(kb_is_key_down(KEY_P))
                game_state.is_game_paused = !game_state.is_game_paused;
            }

            if (msg.m_notify.interrupts & BIT(game_state.mouse_bit)) {

              mouse_update_state();
              //print_mouse_state();
            }

            if (msg.m_notify.interrupts & BIT(game_state.timer_bit)) {

              if(!game_state.is_game_paused) {
                // If changing state, skip updates
                if(manager_state_changes() == OK)
                  continue;

                game_state.update(&game_state);
                game_state.draw(&game_state);
              }

              kb_clear_keys_down_up();
              mouse_clear_buttons_down_up();
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

  return OK;
}

int dm_init() {

  memset(&game_state, 0, sizeof(game_state_t));
  game_state.is_game_running = false;

  // Subscribe to timer 0 interruptions
  if(timer_subscribe_int(&(game_state.timer_bit)) != 0) {
    fprintf(stderr, "Failed to subscribe to timer interrupt notifications\n");
    return 1;
  }

  if(rtc_init(&(game_state.rtc_bit)) != OK)
    return -1;

  // Init keyboard manager
  if(kb_init(&(game_state.kb_bit)) != OK)
    return -1;

  // Init mouse manager
  if(mouse_init(&(game_state.mouse_bit)) != OK)
    return -1;

  // Prepare starting state
  game_state.next_state = STATE_CURR;
  //switch_to_gs_hold(&game_state);
  switch_to_gs_menu(&game_state);
  if(game_state.enter)
    game_state.enter(&game_state);

  return OK;
}

void dm_start() {

  if (dm_loop() != OK)
    printf("LOOP END - FAIL");
  else
    printf("LOOP END - OK");


  if(game_state.exit)
    game_state.exit(&game_state);

  printf("%s\n", game_state.error_msg);
}

void dm_destroy() {

  kb_destroy();

  mouse_destroy();

  rtc_destroy();

  int err = timer_unsubscribe_int();
  if(err != OK)
    printf("Bad timer unsubscribe\n");

  // empty KBC_OUT_BUF
  kbc_ih();
}
