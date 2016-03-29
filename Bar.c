#ifndef BAR_C
#define BAR_C

#include "Bar.h"

// const Bar Bar_default = {0, FLOOR - 10 - 3};

void updateBar(Bar* bar, BarMovementCode movementCode) {

	int new_x;

	switch(movementCode) {

		case BAR_MOVE_LEFT :
		new_x = bar->x - MOVE_SPEED;
		if(new_x < LEFT_WALL + BAR_WIDTH/2) {
			bar->x = LEFT_WALL + BAR_WIDTH/2;
		} else {
			bar->x = new_x;
		}
		break;

		case BAR_MOVE_RIGHT :
		new_x = bar->x + MOVE_SPEED;
		if(new_x > RIGHT_WALL - BAR_WIDTH/2) {
			bar->x = RIGHT_WALL - BAR_WIDTH/2;
		} else {
			bar->x = new_x;
		}
		break;

		case BAR_JUMP_LEFT :
		new_x = bar->x - JUMP_SPEED;
		if(new_x < LEFT_WALL + BAR_WIDTH/2) {
			bar->x = LEFT_WALL + BAR_WIDTH/2;
		} else {
			bar->x = new_x;
		}
		break;

		case BAR_JUMP_RIGHT :
		new_x = bar->x + JUMP_SPEED;
		if(new_x > RIGHT_WALL - BAR_WIDTH/2) {
			bar->x = RIGHT_WALL - BAR_WIDTH/2;
		} else {
			bar->x = new_x;
		}
		break;

		case BAR_NO_MOVEMENT :
		break;

		default :
		break;
	}
}
#endif
