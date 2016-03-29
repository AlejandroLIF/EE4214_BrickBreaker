#include "collisions.h"

CollisionCode checkCollideBar(Ball* ball, Bar* bar) {

	// Check if the ball is in range of collision

	if(ball->y < BAR_Y - DIAMETER/2) {
		return COLLIDE_NONE;
	}

	if(ball->y > BAR_Y + BAR_HEIGHT/2) {
		return COLLIDE_NONE;
	}

	if(ball->x > bar->x + BAR_WIDTH/2 + DIAMETER/2) {
		return COLLIDE_NONE;
	}

	if(ball->x < bar->x - BAR_WIDTH/2 - DIAMETER/2) {
		return COLLIDE_NONE;
	}

	// Ball is now in range of collision, determine type

	if(ball->x < bar->x) {
		if(ball->x < bar->x - N_REGION_WIDTH/2) {
			if(ball->x < bar->x - N_REGION_WIDTH/2 - S_REGION_WIDTH) {
				return COLLIDE_BAR_AMINUS;
			} else {
				return COLLIDE_BAR_SMINUS;
			}
		} else {
			return COLLIDE_BAR_N;
		}
		
	} else {
		if(ball->x > bar->x + N_REGION_WIDTH/2) {
			if(ball->x > bar->x + N_REGION_WIDTH/2 + S_REGION_WIDTH) {
				return COLLIDE_BAR_APLUS;
			} else {
				return COLLIDE_BAR_SPLUS;
			}
		} else {
			return COLLIDE_BAR_N;
		}
	}
}

CollisionCode checkCollideWall(Ball* ball) {
	if(ball->x >= RIGHT_WALL - DIAMETER/2) {
		return COLLIDE_WALL_RIGHT;
	}

	if(ball->x <= LEFT_WALL + DIAMETER/2) {
		return COLLIDE_WALL_LEFT;
	}

	if(ball->y <= CEIL + DIAMETER/2) {
		return COLLIDE_WALL_CEIL;
	}

	if(ball->y >= FLOOR + DIAMETER/2) {
		return COLLIDE_WALL_FLOOR;
	}
}


/*     UL         UC          UR
     _______|___________|__________
     __CL___|___BRICK___|_____CR___
            |           |       
       BL         BC          BR
*/

CollisionCode checkCollideBrick(Ball* ball, Brick* brick) {

	if(ball->x < brick->x) {
		//To the left of vertical center line
		//Possible collisions: UL, UC, CL, BL, BC
		if(ball->y < brick->y) {
			//Above horizontal center line
			//Possible collisions: UL, UC, CL
			if(ball->x < brick->x - BRICK_WIDTH/2) {
				//To the left of brick
				//Possible collisions: UL, CL
				if(ball->y < brick->y - BRICK_HEIGHT/2) {
					//Above brick
					//Possible collisions: UL
					return COLLIDE_BRICK_UL;
				} else {
					//Within brick height
					//Possible collisions: CL
					return COLLIDE_BRICK_CL;
				}
			} else {
				//Within brick width
				//Possible collisions: UC
				return COLLIDE_BRICK_UC;
			}
		} else {
			//Below horizontal center line
			//Possible collisions: CL, BL, BC
			if(ball->x < brick->x - BRICK_WIDTH/2) {
				//To the left of brick
				//Possible collisions: CL, BL
				if(ball->y > brick->y + BRICK_HEIGHT/2) {
					//Below brick
					//Possible collisions: BL
					return COLLIDE_BRICK_BL;
				} else {
					//Within brick height
					//Possible collisions: CL
					return COLLIDE_BRICK_CL
				}
			} else {
				//Within brick width
				//Possible collisions: BC
				return COLLIDE_BRICK_BC;
			}
		}
	} else {
		//To the right of vertical center line
		//Possible collisions: UC, UR, CR, BC, BR
		if(ball->y < brick->y) {
			//Above horizontal center line
			//Possible collisions: UC, UR, CR
			if(ball->x > brick->x + BRICK_WIDTH/2) {
				//To the right of brick
				//Possible collisions: UR, CR
				if(ball->y < brick->y - BRICK_HEIGHT/2) {
					//Above brick
					//Possible collisions: UR
					return COLLIDE_BRICK_UR;
				} else {
					//Within height of brick
					//Possible collisions: CR
					return COLLIDE_BRICK_CR;
				}
			} else {
				//Within width of brick
				//Possible collisions: UC
				return COLLIDE_BRICK_UC;
			}
		} else {
			//Below horizontal center line
			//Possible collisions: CR, BC, BR
			if(ball->x > brick->x + BRICK_WIDTH/2) {
				//To the right of brick
				//Possible collisions: CR, BR
				if(ball->y > brick->y + BRICK_HEIGHT/2) {
					//Below brick
					//Possible collisions: BR
					return COLLIDE_BRICK_BR;
				} else {
					//Within height of brick
					//Possible collisions: CR
					return COLLIDE_BRICK_CR;
				}
			} else {
				//Within width of brick
				//Possible collisions: BC
				return COLLIDE_BRICK_BC;
			}
		}
	}
}
