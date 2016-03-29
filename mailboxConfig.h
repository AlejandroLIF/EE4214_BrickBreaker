
#include "xmbox.h"

#define MBOX_DEVICE_ID      XPAR_MBOX_0_DEVICE_ID
#define MBOX_SIGNAL_RESTART 0xFFFFFFFF

typedef enum{
    MBOX_MSG_RESTART,
    MBOX_MSG_BEGIN_COMPUTATION,
    MBOX_MSG_DRAW_BRICK,
    MBOX_MSG_COLLISION,
    MBOX_MSG_BALL,
    MBOX_MSG_END_COMPUTATION
} MBOX_MSG_TYPE;

typedef enum{
    MBOX_MSG_ID_SIZE = 1,
    MBOX_MSG_DRAW_BRICK_SIZE = 3,
    MBOX_MSG_COLLISION_SIZE = 1,
    MBOX_MSG_BALL_SIZE = 2
} MBOX_MSG_TYPE_SIZE;

//The Mailbox allows communication between the two cores to take place
static XMbox mailbox;
