#ifndef __GS_WOLFENSTEIN_H__
#define __GS_WOLFENSTEIN_H__

#include "game_state.h"

/**
 * @brief Change current game state function pointers to
 * the ones belonging to this state
 * 
 * @param gs Pointer to the current game state struct
 */
void switch_to_gs_wolfenstein(game_state_t* gs);

#endif /* __GS_WOLFENSTEIN_H__ */
