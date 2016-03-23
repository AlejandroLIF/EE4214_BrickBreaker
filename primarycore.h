#ifndef PRIMARY_CORE_H
#define PRIMARY_CORE_H

#include "xmk.h"
#include "xmbox.h"
#include "xstatus.h"
#include <pthread.h>
#include <errno.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/timer.h>
#include <xparameters.h>
#include "semaphore.h"

#include "Ball.h"
#include "Bar.h"
#include "Brick.h"

#define MBOX_DEVICE_ID      XPAR_MBOX_0_DEVICE_ID
#define MBOX_SIGNAL_RESTART 0xFFFFFFFF
#define MBOX_DATA_SIZE      2   //FIXME: we should send a fixed size to the mailbox.
#define SLEEPCONSTANT       40
#define SEM_SHARED          1
#define SEM_PRIVATE         0
#define SEM_AVAILABLE       1
#define SEM_BLOCKED         0



typedef enum{
    MSGQ_ID_BALL,
    MSGQ_ID_BAR,
    MSGQ_ID_BRICK
} MSGQ_ID;

typedef enum{
    MSG_TYPE_BALL,
    MSG_TYPE_BAR,
    MSG_TYPE_BRICK
} MSG_TYPE;

typedef enum{
    BUTTON_CENTER   = (1u << 0), //1
    BUTTON_DOWN     = (1u << 1), //2
    BUTTON_LEFT     = (1u << 2), //4
    BUTTON_RIGHT    = (1u << 3), //8
    BUTTON_UP       = (1u << 4), //16
} BUTTON_CODES;

typedef enum{
    BAR_NO_MOVEMENT,
    BAR_MOVE_LEFT,
    BAR_MOVE_RIGHT,
    BAR_JUMP_LEFT,
    BAR_JUMP_RIGHT
} BAR_MOVEMENT_CODES;

static volatile int buttonInput,
                    dead,
                    win,
                    launch,
                    paused,
                    loseLife,
                    barMovementCode,
                    brickUpdateComplete;

//Main game component (Ball and Bar) declarations.
static Bar bar;
static Ball ball;
//The Mailbox allows communication between the two cores to take place
static XMbox mailbox;
//pthread pointers
static pthread_t pthread_mainLoop,
                 pthread_welcome,
                 pthread_ready,
                 pthread_running,
                 pthread_loseLife,
                 pthread_gameOver,
                 pthread_win,
                 pthread_updateBar,
                 pthread_updateBall,
                 pthread_drawGameArea,
                 pthread_brickCollisionListener,
                 pthread_brickUpdateCompleteListener,
                 pthread_drawStatusArea;

static pthread_attr_t attr;
struct sched_param schedpar;
static sem_t sem_running,
             sem_drawGameArea,
             sem_brickCollisionListener,
             sem_brickUpdateCompleteListener;

//Firmware entry point
int main(void);
//Xilkernel entry point
int main_prog(void);

void queueDraw(const MSG_TYPE msgType, void* data, int size);

//Game state threads
void* thread_mainLoop(void);
void* thread_welcome(void);
void* thread_ready(void);
void* thread_running(void);
void* thread_loseLife(void);
void* thread_gameOver(void);
void* thread_win(void);
void* thread_drawGameArea(void);
void* thread_drawStatusArea(void);
void* thread_brickCollisionListener(void);
void* thread_brickUpdateCompleteListener(void);

//Running state threads
void* updateBar(void);
void* updateBall(void);

#endif
