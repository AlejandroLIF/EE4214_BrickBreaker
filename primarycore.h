#ifndef PRIMARY_CORE_H
#define PRIMARY_CORE_H

#include "xmk.h"
#include "xstatus.h"
#include <pthread.h>
#include <errno.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/timer.h>
#include "semaphore.h"

#include "xtftConfig.h"
#include "buttonIO.h"
#include "MainScreen.h"
#include "mailboxConfig.h"
#include "Ball.h"
#include "Bar.h"
#include "Brick.h"

#define INITIAL_LIVES       3
#define SLEEPCONSTANT       40
#define SEM_SHARED          1
#define SEM_PRIVATE         0
#define SEM_AVAILABLE       1
#define SEM_BLOCKED         0

typedef enum{
    MSGQ_MSGSIZE_BALL = 3,
    MSGQ_MSGSIZE_BAR = 2,
    MSGQ_MSGSIZE_BRICK = 3,
    MSGQ_MSGSIZE_BRICK_COLLISION = 2
} MSGQ_MSGSIZE;

typedef enum{
    MSGQ_TYPE_BALL,
    MSGQ_TYPE_BAR,
    MSGQ_TYPE_BRICK,
    MSGQ_TYPE_BRICK_COLLISION
} MSGQ_TYPE;

static volatile int buttonInput,
                    win,
                    paused,
                    loseLife,
                    barMovementCode,
                    brickUpdateComplete;

//Main game component (Ball and Bar) declarations.
static Bar bar;
static Ball ball;
static unsigned int lives;
static unsigned int score;

static XTft TftInstance;

//pthread pointers
static pthread_t pthread_mainLoop,
                 pthread_drawGameArea,
                 pthread_brickCollisionListener,
                 pthread_mailboxListener,
                 pthread_drawStatusArea;

static pthread_attr_t attr;
struct sched_param schedpar;
static sem_t sem_running,
             sem_drawGameArea,
             sem_drawStatusArea,
             sem_brickCollisionListener,
             sem_mailboxListener;

//Firmware entry point
int main(void);
//Xilkernel entry point
int main_prog(void);

//Game state threads
void* thread_mainLoop(void);
void* thread_drawGameArea(void);
void* thread_drawStatusArea(void);
void* thread_brickCollisionListener(void);
void* thread_mailboxListener(void);

//Running state methods
void queueDraw(const MSGQ_TYPE msgType, void* data, const MSGQ_MSGSIZE size);
void welcome(void);
void ready(void);
void running(void);
void gameOver(void);
void gameWin(void);
int readFromMessageQueue(const MSGQ_TYPE id, void* dataBuffer, const MSGQ_MSGSIZE size);
void draw(unsigned int* dataBuffer, const MSGQ_TYPE msgType);

//Interrupt handler thread
static void gpPBIntHandler(void *arg);
#endif
