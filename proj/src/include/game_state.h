#ifndef __GAME_STATE_H__
#define __GAME_STATE_H__

#include <stdint.h>
#include <stdbool.h>

#include "vector2d.h"

#define STATE_CURR 0
#define STATE_HOLD 1
#define STATE_RUNNER 2
#define STATE_WOLFENSTEIN 3
#define STATE_MENU 4

/**
 * @brief game state struct that holds current state function pointers 
 * and other information
 * 
 */
typedef struct game_state game_state_t;
struct game_state {

  void(* enter) (game_state_t*);    /**< Pointer to current state enter function */
  void(* update) (game_state_t*);   /**< Pointer to current state update function */
  void(* draw) (game_state_t*);     /**< Pointer to current state draw function */
  void(* exit) (game_state_t*);     /**< Pointer to current state exit function */

  uint8_t timer_bit;                /**< Bit used for timer interrupts */
  uint8_t kb_bit;                   /**< Bit used for keyboard interrupts */
  uint8_t mouse_bit;                /**< Bit used for mouse interrupts */
  uint8_t rtc_bit;                  /**< Bit used for rtc interrupts */

  bool is_game_running;             /**< Flag to control interrupt cycle */
  bool is_game_paused;              /**< Flag to control game state update/draw logic */
  uint8_t next_state;               /**< Flag to control game state switching */
  char error_msg[256];              /**< String for inter-state */
};

#endif /* __GAME_STATE_H__ */
