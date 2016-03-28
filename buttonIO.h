
#include "xgpio.h"

typedef enum{
    BUTTON_CENTER   = (1u << 0), //1
    BUTTON_DOWN     = (1u << 1), //2
    BUTTON_LEFT     = (1u << 2), //4
    BUTTON_RIGHT    = (1u << 3), //8
    BUTTON_UP       = (1u << 4), //16
} BUTTON_CODES;
