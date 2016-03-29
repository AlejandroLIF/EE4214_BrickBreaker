#ifndef BALL_C
#define BALL_C

#include "Ball.h"

// const Ball Ball_default = {0, 0, 0, 10, 0}; //FIXME: set ball default

const unsigned int BALL_MASK[DIAMETER][DIAMETER] = {
                                {NONE,NONE,NONE,NONE,NONE,FILL,FILL,FILL,FILL,FILL,NONE,NONE,NONE,NONE,NONE},
                                {NONE,NONE,NONE,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,NONE,NONE,NONE},
                                {NONE,NONE,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,NONE,NONE},
                                {NONE,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,NONE},
                                {NONE,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,NONE},
                                {FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL},
                                {FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL},
                                {FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL},
                                {FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL},
                                {FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL},
                                {NONE,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,NONE},
                                {NONE,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,NONE},
                                {NONE,NONE,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,NONE,NONE},
                                {NONE,NONE,NONE,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,NONE,NONE,NONE},
                                {NONE,NONE,NONE,NONE,NONE,FILL,FILL,FILL,FILL,FILL,NONE,NONE,NONE,NONE,NONE}
                            };

void updateBallPosition(Ball* ball){
    int speed = ball->s;
    int dir = ball->d;
    int old_x = ball->x;
    int old_y = ball->y;

    int new_x = speed * SIN_TABLE[ (dir+90) % 360 ] + old_x;
    int new_y = speed * SIN_TABLE[ dir ] + old_y;

    if(new_x > RIGHT_WALL - DIAMETER/2) {
    	new_x = RIGHT_WALL - DIAMETER/2;
    } else if(new_x < LEFT_WALL + DIAMETER/2) {
    	new_x = LEFT_WALL + DIAMETER/2;
    }

    if(new_y < CEIL + DIAMETER/2) {
    	new_y = CEIL + DIAMETER/2;
    } else if(new_y > FLOOR - DIAMETER/2) {
    	new_y = FLOOR - DIAMETER/2;
    }

    ball->x = new_x;
    ball->y = new_y;

}

void followBar(Ball* ball, Bar* bar){

    ball->x = bar->x;

}

#endif
