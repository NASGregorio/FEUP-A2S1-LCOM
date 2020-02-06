#ifndef __GS_INF_RUNNER_H__
#define __GS_INF_RUNNER_H__

#include "game_state.h"

/**
 * @brief Change current game state function pointers to
 * the ones belonging to this state
 * 
 * @param gs Pointer to the current game state struct
 */
void switch_to_gs_runner(game_state_t* gs);

#endif /* __GS_INF_RUNNER_H__ */
