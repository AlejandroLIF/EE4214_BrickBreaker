#ifndef BRICK_H
#define BRICK_H

#define BRICK_WIDTH     41
#define BRICK_HEIGHT    15
#define BRICK_SPACING   5

#define BRICK_COLUMN_HEIGHT 8

#define BRICK_SIZE      12;

struct Brick_s{
    int x; /*horizontal anchor*/
    int y; /*vertical anchor*/
    int c;       /*color*/
};

typedef struct Brick_s Brick;

#endif
