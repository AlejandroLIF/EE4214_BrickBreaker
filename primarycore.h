#ifndef PRIMARY_CORE_H
#define PRIMARY_CORE_H

#include "xmk.h"
#include "xstatus.h"
#include <pthread.h>
#include <errno.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/timer.h>
#include <sys/intr.h>
#include "semaphore.h"

#include "xtftConfig.h"
#include "xmutexConfig.h"

#include "buttonIO.h"
#include "MainScreen.h"
#include "mailboxConfig.h"
#include "Ball.h"
#include "Bar.h"
#include "Brick.h"
#include "collisions.h"

#define INITIAL_LIVES       3
#define SEM_SHARED          1
#define SEM_PRIVATE         0
#define SEM_AVAILABLE       1
#define SEM_BLOCKED         0
#define BRICK_SCORE_NORMAL  1
#define BRICK_SCORE_GOLDEN  2
#define SCORE_MILESTONE     10
#define FPS					50
#define	CLK_PERIOD_MS		1
#define PERIOD_TICKS        1000/FPS
#define JUMP_CYCLE_THRESHOLD 1000
#define DEBOUNCE_CYCLES		4

typedef enum{
    MSGQ_MSGSIZE_BALL =             3 * sizeof(int),
    MSGQ_MSGSIZE_BAR =              3 * sizeof(int),
    MSGQ_MSGSIZE_BRICK =            3 * sizeof(int),
    MSGQ_MSGSIZE_BACKGROUND =       1 * sizeof(int),
    MSGQ_MSGSIZE_BRICK_COLLISION =  2 * sizeof(int),
    MSGQ_MSGSIZE_GAMEAREA =         1 * sizeof(int),
    MSGQ_MSGSIZE_STATUSAREA =       1 * sizeof(int),
    MSGQ_MSGSIZE_DRAWTIME =         1 * sizeof(int)
} MSGQ_MSGSIZE;

typedef enum{
    MSGQ_TYPE_BALL,
    MSGQ_TYPE_BAR,
    MSGQ_TYPE_BRICK,
    MSGQ_TYPE_BRICK_COLLISION,
    MSGQ_TYPE_BACKGROUND,
    MSGQ_TYPE_GAMEAREA,
    MSGQ_TYPE_STATUSAREA,
    MSGQ_TYPE_DRAWTIME
} MSGQ_TYPE;

static volatile int buttonInput,
                    win,
                    paused,
                    loseLife,
                    barMovementCode,
                    brickUpdateComplete;

//Main game component (Ball and Bar) declarations.
extern Bar bar;
extern Ball ball;
static volatile unsigned int lives;
static volatile unsigned int score;
static volatile unsigned int last_drawn_lives;
static volatile unsigned int last_drawn_score;
static volatile unsigned int last_drawn_speed;
static volatile unsigned int hasCollided;
static volatile int nextScoreMilestone;
static volatile int scoreMilestoneReached;
static volatile unsigned int lastInterrupt;
static volatile unsigned int gameCycles;
static time_t ticks_before,
                    ticks_diff;

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
             sem_mailboxListener,
             sem_fps_start,
             sem_fps_end,
             sem_btn_start,
             sem_btn_end;

 //Running state methods
 void queueMsg(const MSGQ_TYPE msgType, void* data, const MSGQ_MSGSIZE size);
 void welcome(void);
 void ready(void);
 void running(void);
 void gameOver(void);
 void gameWin(void);
 int readFromMessageQueue(const MSGQ_TYPE id, void* dataBuffer, const MSGQ_MSGSIZE size);
 void draw(unsigned int* dataBuffer, const MSGQ_TYPE msgType);
 void safePrint(const char *ptr);
 inline void buildBallMessage(Ball* ball, unsigned int* message);
 int increaseScore(int isGoldenBrick);
 unsigned char intToChar(int n);
 void screenWrite(char* str, int size);
 void resetBallAndBar(void);
 void eraseBar(Bar* bar);
 void eraseBall(Ball* ball);
 void drawBar(Bar* bar);
 void drawBall(Ball* ball);

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
void* thread_fpsTimer(void);
void* thread_btnTimer(void);


//Interrupt handler thread
static void gpPBIntHandler(void *arg);
#endif
