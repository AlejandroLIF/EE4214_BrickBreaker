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

const double SIN_TABLE[360] = {
                                0.000,  0.017, 0.035, 0.052, 0.070, 0.087, 0.105, 0.122, 0.139, 0.156, 0.174, 0.191, 0.208, 0.225, 0.242, 0.259,
                                0.276, 0.292, 0.309, 0.326, 0.342, 0.358, 0.375, 0.391, 0.407, 0.423, 0.438, 0.454, 0.469, 0.485, 0.500, 0.515,
                                0.530, 0.545, 0.559, 0.574, 0.588, 0.602, 0.616, 0.629, 0.643, 0.656, 0.669, 0.682, 0.695, 0.707, 0.719, 0.731,
                                0.743, 0.755, 0.766, 0.777, 0.788, 0.799, 0.809, 0.819, 0.829, 0.839, 0.848, 0.857, 0.866, 0.875, 0.883, 0.891,
                                0.899, 0.906, 0.914, 0.921, 0.927, 0.934, 0.940, 0.946, 0.951, 0.956, 0.961, 0.966, 0.970, 0.974, 0.978, 0.982,
                                0.985, 0.988, 0.990, 0.993, 0.995, 0.996, 0.998, 0.999, 0.999, 1.000, 1.000, 1.000, 0.999, 0.999, 0.998, 0.996,
                                0.995, 0.993, 0.990, 0.988, 0.985, 0.982, 0.978, 0.974, 0.970, 0.966, 0.961, 0.956, 0.951, 0.946, 0.940, 0.934,
                                0.927, 0.921, 0.914, 0.906, 0.899, 0.891, 0.883, 0.875, 0.866, 0.857, 0.848, 0.839, 0.829, 0.819, 0.809, 0.799,
                                0.788, 0.777, 0.766, 0.755, 0.743, 0.731, 0.719, 0.707, 0.695, 0.682, 0.669, 0.656, 0.643, 0.629, 0.616, 0.602,
                                0.588, 0.574, 0.559, 0.545, 0.530, 0.515, 0.500, 0.485, 0.469, 0.454, 0.438, 0.423, 0.407, 0.391, 0.375, 0.358,
                                0.342, 0.326, 0.309, 0.292, 0.276, 0.259, 0.242, 0.225, 0.208, 0.191, 0.174, 0.156, 0.139, 0.122, 0.105, 0.087,
                                0.070, 0.052, 0.035, 0.017, 0.000, -0.017, -0.035, -0.052, -0.070, -0.087, -0.105, -0.122, -0.139, -0.156, -0.174,
                                -0.191, -0.208, -0.225, -0.242, -0.259, -0.276, -0.292, -0.309, -0.326, -0.342, -0.358, -0.375, -0.391, -0.407,
                                -0.423, -0.438, -0.454, -0.469, -0.485, -0.500, -0.515, -0.530, -0.545, -0.559, -0.574, -0.588, -0.602, -0.616,
                                -0.629, -0.643, -0.656, -0.669, -0.682, -0.695, -0.707, -0.719, -0.731, -0.743, -0.755, -0.766, -0.777, -0.788,
                                -0.799, -0.809, -0.819, -0.829, -0.839, -0.848, -0.857, -0.866, -0.875, -0.883, -0.891, -0.899, -0.906, -0.914,
                                -0.921, -0.927, -0.934, -0.940, -0.946, -0.951, -0.956, -0.961, -0.966, -0.970, -0.974, -0.978, -0.982, -0.985,
                                -0.988, -0.990, -0.993, -0.995, -0.996, -0.998, -0.999, -0.999, -1.000, -1.000, -1.000, -0.999, -0.999, -0.998,
                                -0.996, -0.995, -0.993, -0.990, -0.988, -0.985, -0.982, -0.978, -0.974, -0.970, -0.966, -0.961, -0.956, -0.951,
                                -0.946, -0.940, -0.934, -0.927, -0.921, -0.914, -0.906, -0.899, -0.891, -0.883, -0.875, -0.866, -0.857, -0.848,
                                -0.839, -0.829, -0.819, -0.809, -0.799, -0.788, -0.777, -0.766, -0.755, -0.743, -0.731, -0.719, -0.707, -0.695,
                                -0.682, -0.669, -0.656, -0.643, -0.629, -0.616, -0.602, -0.588, -0.574, -0.559, -0.545, -0.530, -0.515, -0.500,
                                -0.485, -0.469, -0.454, -0.438, -0.423, -0.407, -0.391, -0.375, -0.358, -0.342, -0.326, -0.309, -0.292, -0.276,
                                -0.259, -0.242, -0.225, -0.208, -0.191, -0.174, -0.156, -0.139, -0.122, -0.105, -0.087, -0.070, -0.052, -0.035,
                                -0.017
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
