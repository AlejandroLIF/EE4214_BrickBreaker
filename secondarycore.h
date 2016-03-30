#ifndef SECONDARYCORE_H
#define SECONDARYCORE_H

#include "xmk.h"
#include <xparameters.h>
#include "xmbox.h"
#include "xstatus.h"
#include <pthread.h>
#include <errno.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/timer.h>
#include <stdlib.h>

#include "Ball.h"
#include "Brick.h"
#include "MainScreen.h"
#include "mailboxConfig.h"
#include "collisions.h"

#define MY_CPU_ID XPAR_CPU_ID
#define MBOX_DEVICE_ID		XPAR_MBOX_0_DEVICE_ID

#define ACTIVE              1
#define INACTIVE            0
#define MAX_GOLDEN_COLUMNS  2
#define COLUMNS             10
#define ROWS                8
#define SEM_SHARED          1
#define SEM_PRIVATE         0
#define SEM_AVAILABLE       1
#define SEM_BLOCKED         0

static XMbox mailbox;
static int activeBricks[COLUMNS][ROWS];
static int bricksLeft[COLUMNS];
static int goldenColumn[COLUMNS];
static int columnsLeft;

static pthread_attr_t attr;
struct sched_param schedpar;
static sem_t sem_goldenColumns,
             sem_updateComplete,
             sem_columnStart,
             sem_columnWait;

static Ball ball;

static pthread_t pthread_mailboxListener,
                 pthread_updateComplete,
                 pthread_goldenSelector,
                 pthread_col0,
                 pthread_col1,
                 pthread_col2,
                 pthread_col3,
                 pthread_col4,
                 pthread_col5,
                 pthread_col6,
                 pthread_col7,
                 pthread_col8,
                 pthread_col9;

int main(void);
int main_prog(void);

static inline int makeGolden(void);

void columnCode(const int colID);
void* thread_mailboxListener(void);
void* thread_updateComplete(void);
void* thread_goldenSelector(void);
void* thread_col0(void);
void* thread_col1(void);
void* thread_col2(void);
void* thread_col3(void);
void* thread_col4(void);
void* thread_col5(void);
void* thread_col6(void);
void* thread_col7(void);
void* thread_col8(void);
void* thread_col9(void);
Brick toBrick(int colID, int bricknum);

#endif
