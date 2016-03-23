#define BRICK_WIDTH     40
#define BRICK_HEIGHT    15
#define BRICK_SPACING   5
#define BRICK_SIZE      12;

struct Brick_s{
    const int x; /*horizontal anchor*/
    const int y; /*vertical anchor*/
    int c;       /*color*/
};

typedef struct Brick_s Brick;
