#include <lcom/lcf.h>
#include <stdlib.h>

#include "gesture_logic_and.h"
//#include "i8042.h"

uint16_t mean_dx = 0;
uint16_t mean_dy = 0;

int track_logic_and(gesture_state_t* state, uint8_t x_len, uint8_t tolerance, 
									const mouse_packet_t* current_mouse_packet) {

	if (state == NULL || current_mouse_packet == NULL)
		return GESTURE_ERROR;

	int16_t dx = current_mouse_packet->delta_x;
	int16_t dy = current_mouse_packet->delta_y;
	uint8_t btn;
	
	switch (*state)
	{
	case Gesture_Start:
		btn = check_mouse_btn_state(current_mouse_packet);
		if(btn == MS_LBTN) {
			mean_dx = mean_dy = 0;
			*state = Gesture_LB_Down;
		}
		break;

	case Gesture_LB_Down:
		btn = check_mouse_btn_state(current_mouse_packet);
		if(btn == 0) {
				*state = (mean_dx >= x_len && abs(mean_dy) > abs(mean_dx)) ? Gesture_LB_Up : Gesture_Start;
		}
		else {
			mean_dx += dx;
			mean_dy += dy;
			if(dx < -tolerance || dy < -tolerance || btn != MS_LBTN)
				*state = Gesture_Start;
		}
		break;

	case Gesture_LB_Up:
		btn = check_mouse_btn_state(current_mouse_packet);
		if(btn == MS_RBTN) {
			mean_dx = mean_dy = 0;
			*state = Gesture_RB_Down;
		}
		else if(btn == MS_LBTN) {
			*state = Gesture_LB_Down;
		}
		else if(abs(dx) > tolerance || abs(dy) > tolerance || btn != 0)
			*state = Gesture_Start;
		break;

	case Gesture_RB_Down:
		btn = check_mouse_btn_state(current_mouse_packet);
		if(btn == 0) {
			if(mean_dx >= x_len && abs(mean_dy) > abs(mean_dx))
				return GESTURE_COMPLETE;
			else
				*state = Gesture_Start;
		}
		else {
			mean_dx += dx;
			mean_dy += dy;
			if(dx < -tolerance || dy > tolerance || btn != MS_RBTN) {
				*state = Gesture_Start;
			}
		}
		break;

	case Gesture_RB_Up:
		return GESTURE_COMPLETE;

	default:
		return GESTURE_ERROR;
	}

	return GESTURE_INCOMPLETE;
}
