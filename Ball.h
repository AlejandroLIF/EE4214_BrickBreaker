#ifndef BALL_H
#define BALL_H

#include "Bar.h"
#include "MainScreen.h"

#define FILL        0xFFFFFFFF
#define NONE        0X00000000
#define DIAMETER    15
#define BALL_SIZE   20
#define BALL_COLOR  0x00151515
#define MAX_SPEED	1000
#define MIN_SPEED	50

struct Ball_s{
    	unsigned int x; /*horizontal anchor*/
    	unsigned int y; /*vertical anchor*/
        unsigned int c; /*color*/
    	unsigned int d; /*direction*/
    	unsigned int s; /*speed*/
};
typedef struct Ball_s Ball;

// extern const Ball Ball_default;
extern const unsigned int BALL_MASK[DIAMETER][DIAMETER];
extern const double SIN_TABLE[360];

void updateBallPosition(Ball* ball);
void followBar(Ball* ball, Bar* bar);
#endif
