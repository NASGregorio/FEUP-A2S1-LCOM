#ifndef __GESTURE_LOGIC_AND__
#define __GESTURE_LOGIC_AND__

#include "mouse.h"

#define GESTURE_COMPLETE 0
#define GESTURE_INCOMPLETE 1
#define GESTURE_ERROR -1

typedef enum {
	Gesture_Start,
	Gesture_LB_Down,
	Gesture_LB_Up,
	Gesture_RB_Down,
	Gesture_RB_Up,
	Gesture_Error
} gesture_state_t; 

int track_logic_and(gesture_state_t* state, uint8_t x_len, uint8_t tolerance, 
							const mouse_packet_t* current_mouse_packet);

#endif /* __GESTURE_LOGIC_AND__ */
