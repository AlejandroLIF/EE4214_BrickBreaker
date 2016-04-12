#ifndef COLLISIONS_C
#define COLLISION_C
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
    if(ball->d > 0 && ball->d < 180) {
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
    } else {
        return COLLIDE_NONE;
    }
}

CollisionCode checkCollideWall(Ball* ball) {
	if(ball->x >= RIGHT_WALL - DIAMETER/2 && (ball->d < 90 || ball->d > 270)) {
		return COLLIDE_WALL_RIGHT;
	}

	if(ball->x <= LEFT_WALL + DIAMETER/2 && (ball->d > 90 && ball->d < 270)) {
		return COLLIDE_WALL_LEFT;
	}

	if(ball->y <= CEIL + DIAMETER/2 && (ball->d > 180 && ball->d < 360) && ball->d != 0) {
		return COLLIDE_WALL_CEIL;
	}

	if(ball->y >= FLOOR - DIAMETER/2) {
		return COLLIDE_WALL_FLOOR;
	}
	return COLLIDE_NONE;
}


/*     UL         UC          UR
     _______|___________|__________
     __CL___|___BRICK___|_____CR___
            |           |
       BL         BC          BR
*/

//TODO Determine whether or not corner collision is at all possible using below
//     May have to change some < or > to <= or >= for the case to be included
CollisionCode checkCollideBrick(Ball* ball, Brick* brick) {
	//Check proximity of ball to brick
	if(ball->x < brick->x - BRICK_WIDTH/2 - DIAMETER/2) {
		//Ball is to the left of brick
		return COLLIDE_NONE;
	}

	if(ball->x > brick->x + BRICK_WIDTH/2 + DIAMETER/2) {
		//Ball is to the right of brick
		return COLLIDE_NONE;
	}

	if(ball->y < brick->y - BRICK_HEIGHT/2 - DIAMETER/2) {
		//Ball is above brick
		return COLLIDE_NONE;
	}

	if(ball->y > brick->y + BRICK_HEIGHT/2 + DIAMETER/2) {
		//Ball is below brick
		return COLLIDE_NONE;
	}

	//Ball is now hitting the brick, check where
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
					return COLLIDE_BRICK_CL;
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

void updateBallDirection(Ball* ball, CollisionCode collision) {
	switch (collision) {
		case COLLIDE_BAR_AMINUS :
        //Should be 360 - ball->d +- 15, maybe depending on which direction the ball is incoming from
			ball->d = 360 - ball->d - 15;
			break;

		case COLLIDE_BAR_SMINUS :
			updateBallSpeed(ball, BALL_SPEED_S_MINUS);
			ball->d = 360 - ball->d;
			break;

		case COLLIDE_BAR_N :
			ball->d = 360 - ball->d;
			break;

		case COLLIDE_BAR_SPLUS :
			updateBallSpeed(ball, BALL_SPEED_S_PLUS);
			ball->d = 360 - ball->d;
			break;

	    case COLLIDE_BAR_APLUS :
        //Should be 360 - ball->d +- 15, maybe depending on which direction the ball is incoming from
		    ball->d = (360 - ball->d + 15) % 360;
		    break;
		case COLLIDE_WALL_LEFT :
            ball->d = ball->d > 180 ? 540 - ball->d : ((540 - ball->d) % 360);
            break;
		case COLLIDE_WALL_RIGHT : //TODO: These cases are the same?
            ball->d = ball->d > 90 ? 540 - ball->d : ((540 - ball->d) % 360);
            break;
		case COLLIDE_WALL_CEIL :
		case COLLIDE_WALL_FLOOR :
			ball->d = 360 - ball->d;
			break;
      //TODO: Implement brick collision bounce
      case COLLIDE_BRICK_UL :
          //Collide upper left corner
          if(ball->d > 315 && ball->d < 360) {
              ball->d = 630 - ball->d;
          } else if (ball->d < 135) {
              ball->d = 270 - ball->d;
          } else {
              ball->d = 360 - ball->d;
          }
          //ball->d = ball->d > 270 ? 630 - ball->d; : 270 - ball->d;
          break;

      case COLLIDE_BRICK_UC :
          //Collide upper center
          ball->d = 360 - ball->d;
          break;

      case COLLIDE_BRICK_UR :
          //Collide upper right corner
          if(ball->d > 45 && ball->d <= 90) {
              ball->d = 90 - ball->d;
          } else if(ball->d < 225 && ball->d > 90) {
              ball->d = 450 - ball->d;
          } else {
              ball->d = 360 - ball->d;
          }
          //ball->d = ball->d > 90 ? 450 - ball->d : 90 - ball->d;
          break;

      case COLLIDE_BRICK_CL :
          //Collide center left
          ball->d = ball->d > 180 ? 540 - ball->d : ((540 - ball->d) % 360);
          break;

      case COLLIDE_BRICK_CR :
          //Collide center right
          ball->d = ball->d > 90 ? 540 - ball->d : ((540 - ball->d) % 360);
          break;

      case COLLIDE_BRICK_BL :
          //Collide bottom left corner
          if(ball->d > 225 && ball->d < 360) {
              ball->d = 450 - ball->d;
          } else if (ball->d < 45) {
              ball->d = 90 - ball->d;
          } else {
              ball->d = 360 - ball->d;
          }
          //ball->d = ball->d > 90 ? 450 - ball->d : 90 - ball->d;
          break;

      case COLLIDE_BRICK_BC :
          //Collide bottom center
          ball->d = 360 - ball->d;
          break;

      case COLLIDE_BRICK_BR :
          //Collide bottom right corner
          if(ball->d > 135 && ball->d <= 270) {
              ball->d = 270 - ball->d;
          } else if (ball->d > 270 && ball->d < 315) {
              ball->d = 630 - ball->d;
          } else {
              ball->d = 360 - ball->d;
          }
          //ball->d = ball->d > 270 ? 630 - ball->d : 270 - ball->d;
          break;

		default:
			break;

	}

}

#endif
