#include "MainScreen.h"

#define BAR_WIDTH       80
#define BAR_HEIGHT      5
#define A_REGION_WIDTH  10
#define S_REGION_WIDTH  10
#define N_REGION_WIDTH  41

#define A_REGION_COLOR 0x00ff0000
#define S_REGION_COLOR 0x000000ff
#define N_REGION_COLOR 0x00000000

#define JUMP_SPEED  25  //This is the rate of positional change in terms of pixels per jump
#define MOVE_SPEED  200 //TODO: adjust this metric to pixels per second, depending on the desired framerate
#define BAR_Y		405

typedef enum{
    BAR_NO_MOVEMENT,
    BAR_MOVE_LEFT,
    BAR_MOVE_RIGHT,
    BAR_JUMP_LEFT,
    BAR_JUMP_RIGHT
} BarMovementCode;

struct Bar_s{
    int x; /*horizontal anchor*/
    const int y; /*vertical anchor*/
} Bar_default = {0, 0};

typedef struct Bar_s Bar;

void updateBar(Bar* bar, BarMovementCode movementCodes);
