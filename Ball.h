#ifndef BALL_H
#define BALL_H

#include "Bar.h"
#include "MainScreen.h"

#define FILL        0xFFFFFFFF
#define NONE        0X00000000
#define DIAMETER    15
#define BALL_SIZE   20
#define BALL_COLOR  0x00151515
#define MAX_SPEED	40              //FIXME: these have to be adjusted to FPS
#define MIN_SPEED	2               //FIXME: these have to be adjusted to FPS
#define BALL_SPEED_SCORE_INCREASE 3 //FIXME: these have to be adjusted to FPS
#define BALL_SPEED_S_MINUS  -10     //FIXME: these have to be adjusted to FPS
#define BALL_SPEED_S_PLUS   10      //FIXME: these have to be adjusted to FPS

struct Ball_s{
    	unsigned int x; /*horizontal anchor*/
    	unsigned int y; /*vertical anchor*/
        unsigned int c; /*color*/
    	unsigned int d; /*direction*/
    	int s; /*speed*/            //XXX: speed is signed to prevent underflow when reducing speed below MIN_SPEED
};
typedef struct Ball_s Ball;

// extern const Ball Ball_default;
extern const unsigned int BALL_MASK[DIAMETER][DIAMETER];
extern const double SIN_TABLE[360];

void updateBallPosition(Ball* ball);
void followBar(Ball* ball, Bar* bar);
void updateBallSpeed(Ball* ball, int amount);
#endif
