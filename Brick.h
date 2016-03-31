#ifndef BRICK_H
#define BRICK_H

#define BRICK_WIDTH     41
#define BRICK_HEIGHT    15
#define BRICK_SPACING   5

#define BRICK_COLUMN_HEIGHT 8

#define BRICK_COLOR_DEFAULT     0x0000FF00 //Green
#define BRICK_COLOR_ACTIVE      0x00FFFF00 //Yellow
#define BRICK_COLOR_DESTROYED   0x00000000 //No color

#define BRICK_SIZE      12;

struct Brick_s{
    int x; /*horizontal anchor*/
    int y; /*vertical anchor*/
    int c;       /*color*/
};

typedef struct Brick_s Brick;

#endif
