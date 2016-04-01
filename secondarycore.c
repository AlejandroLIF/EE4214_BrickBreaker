#include "secondarycore.h"

int main(void){
	xilkernel_init();
	xmk_add_static_thread(main_prog,0);
	xilkernel_start();
	xilkernel_main ();
}

int main_prog(void){
    int status;
    // columnsLeft = 0; //Initialize the secondarycore to a "game over" status //FIXME: WHY? :c
    restart();
    /*BEGIN MAILBOX INITIALIZATION*/
    XMbox_Config *ConfigPtr;
    ConfigPtr = XMbox_LookupConfig(MBOX_DEVICE_ID);
    if(ConfigPtr == (XMbox_Config *)NULL){
        safePrint("Error configuring mailbox uB1 Receiver\r\n");
        return XST_FAILURE;
    }

    status = XMbox_CfgInitialize(&mailbox, ConfigPtr, ConfigPtr->BaseAddress);
    if (status != XST_SUCCESS) {
        safePrint("Error initializing mailbox uB1 Receiver--\r\n");
        return XST_FAILURE;
    }
    /*END MAILBOX INITIALIZATION*/

    /* BEGIN XMUTEX INITIALIZATION */
    status = initXMutex();
    if (status != XST_SUCCESS) {
        return XST_FAILURE;
    }
    /* END XMUTEX INITIALIZATION */

    //Initialize thread semaphores
    if(sem_init(&sem_goldenColumns, SEM_SHARED, MAX_GOLDEN_COLUMNS)){
        safePrint("Error while initializing semaphore sem_goldenColumns!\r\n");
        while(TRUE); //Trap runtime here
    }
    if(sem_init(&sem_updateComplete, SEM_SHARED, SEM_BLOCKED)){
        safePrint("Error while initializing semaphore sem_updateComplete!\r\n");
        while(TRUE); //Trap runtime here
    }
    if(sem_init(&sem_columnStart, SEM_SHARED, SEM_BLOCKED)){
        safePrint("Error while initializing semaphore sem_columnStart!\r\n");
        while(TRUE); //Trap runtime here
    }
    if(sem_init(&sem_columnWait, SEM_SHARED, SEM_BLOCKED)){
        safePrint("Error while initializing semaphore sem_columnWait!\r\n");
        while(TRUE); //Trap runtime here
    }

    pthread_attr_init(&attr);

    schedpar.sched_priority = PRIO_HIGHEST;
    pthread_attr_setschedparam(&attr, &schedpar);
    pthread_create(&pthread_updateComplete, &attr, (void*)thread_updateComplete, NULL);

    schedpar.sched_priority++;
    pthread_attr_setschedparam(&attr, &schedpar);
    pthread_create(&pthread_goldenSelector, &attr, (void*)thread_goldenSelector, NULL);

    schedpar.sched_priority++;
    pthread_attr_setschedparam(&attr, &schedpar);
    while(status = pthread_create(&pthread_col0, &attr, (void*)thread_col0, NULL)){
        safePrint("Error col 0!\r\n");
    }

    // schedpar.sched_priority++;
    pthread_attr_setschedparam(&attr, &schedpar);
    while(status = pthread_create(&pthread_col1, &attr, (void*)thread_col1, NULL)){
        safePrint("Error col 1!\r\n");
    }

    // schedpar.sched_priority++;
    pthread_attr_setschedparam(&attr, &schedpar);
    while(status = pthread_create(&pthread_col2, &attr, (void*)thread_col2, NULL)){
        safePrint("Error col 2!\r\n");
    }

    // schedpar.sched_priority++;
    pthread_attr_setschedparam(&attr, &schedpar);
    while(status = pthread_create(&pthread_col3, &attr, (void*)thread_col3, NULL)){
        safePrint("Error col 3!\r\n");
    }

    // schedpar.sched_priority++;
    pthread_attr_setschedparam(&attr, &schedpar);
    while(status = pthread_create(&pthread_col4, &attr, (void*)thread_col4, NULL)){
        safePrint("Error col 4!\r\n");
    }

    // schedpar.sched_priority++;
    pthread_attr_setschedparam(&attr, &schedpar);
    while(status = pthread_create(&pthread_col5, &attr, (void*)thread_col5, NULL)){
        safePrint("Error col 5!\r\n");
    }

    // schedpar.sched_priority++;
    pthread_attr_setschedparam(&attr, &schedpar);
    while(status = pthread_create(&pthread_col6, &attr, (void*)thread_col6, NULL)){
        safePrint("Error col 6!\r\n");
    }

    // schedpar.sched_priority++;
    pthread_attr_setschedparam(&attr, &schedpar);
    while(status = pthread_create(&pthread_col7, &attr, (void*)thread_col7, NULL)){
        safePrint("Error col 7!\r\n");
    }

    // schedpar.sched_priority++;
    pthread_attr_setschedparam(&attr, &schedpar);
    while(status = pthread_create(&pthread_col8, &attr, (void*)thread_col8, NULL)){
        safePrint("Error col 8!\r\n");
    }

    // schedpar.sched_priority++;
    pthread_attr_setschedparam(&attr, &schedpar);
    while(status = pthread_create(&pthread_col9, &attr, (void*)thread_col9, NULL)){
        safePrint("Error col 9!\r\n");
    }

    schedpar.sched_priority++;
    pthread_attr_setschedparam(&attr, &schedpar);
    while(status = pthread_create(&pthread_mailboxListener, &attr, (void*)thread_mailboxListener, NULL)){
        safePrint("ERROR MAILBOX LISTENER!\r\n");
    }

    return 0;
}

void* thread_mailboxListener(void){
    int dataBuffer[3]; //FIXME: magic number as dataBuffer size
    MBOX_MSG_TYPE msgType;
    int i, j;

    while(TRUE){
        XMbox_ReadBlocking(&mailbox,(u32*)&msgType, MBOX_MSG_ID_SIZE);
        switch(msgType){
            case MBOX_MSG_RESTART:
                restart();
                break;
            case MBOX_MSG_BEGIN_COMPUTATION:
                XMbox_ReadBlocking(&mailbox, (u32*)dataBuffer, MBOX_MSG_BALL_SIZE);
                ball.x = dataBuffer[0];
                ball.y = dataBuffer[1];
                for(i = 0; i < COLUMNS; i++){
                    sem_post(&sem_columnStart);
                }
                break;
            case MBOX_MSG_UPDATE_GOLDEN:
                sem_post(&sem_goldenColumns);
                break;
            default:
                while(TRUE){safePrint("secondary core: mailboxlistener error\r\n");}; //Error! Trap runtime here. THIS SHOULD NOT HAPPEN
        }
    }
}

void restart(void){
	int i, j;
    //Reset the activeBricks array
    for(i = 0; i < COLUMNS; i++){
        for(j = 0; j < ROWS; j++){
            activeBricks[i][j] = TRUE;
        }
        bricksLeft[i] = ROWS;
        goldenColumn[i] = FALSE;
    }

    columnsLeft = COLUMNS;
    for(i = 0; i < MAX_GOLDEN_COLUMNS; i++){
        sem_post(&sem_goldenColumns);
    }
}

static inline int makeGolden(){
	return (rand() % COLUMNS) < MAX_GOLDEN_COLUMNS;
}

void* thread_goldenSelector(void){
    int* goldenPointers[MAX_GOLDEN_COLUMNS];
    int nextPointer = 0;
    int i = 0;

    //Initialize the goldenPointers array to point to columns linearly
    for(i = 0; i < MAX_GOLDEN_COLUMNS; i++){
        goldenPointers[i] = &goldenColumn[i];
    }
    i = 0;
    while(TRUE){
        //Randomly select amongst the columns which still have bricks left.
        if(bricksLeft[i] && makeGolden()){
            sem_wait(&sem_goldenColumns);
            *(goldenPointers[nextPointer]) = FALSE;
            goldenPointers[nextPointer] = &goldenColumn[i];
            *(goldenPointers[nextPointer]) = TRUE;
            nextPointer = (nextPointer + 1) % MAX_GOLDEN_COLUMNS;
        }
        i = (i + 1) % COLUMNS;
    }
}

//Waits for every column thread to signal that it's done to send
void* thread_updateComplete(void){
    const MBOX_MSG_TYPE computationComplete = MBOX_MSG_COMPUTATION_COMPLETE;
    int i;
    while(TRUE){
        for(i = 0; i < COLUMNS; i++){
            sem_wait(&sem_updateComplete);
        }
        XMbox_WriteBlocking(&mailbox, (u32*)&computationComplete, MBOX_MSG_ID_SIZE);
        for(i = 0; i < COLUMNS; i++){
            sem_post(&sem_columnWait);
        }
    }
}

void columnCode(const int colID){
    int i;
    int x = LEFT_WALL + BRICK_SPACING * (colID + 1) + BRICK_WIDTH * (colID + 0.5); //FIXME: this holds the column x position constant. It should not be here.
    Brick b;
    CollisionCode collision;
    int dataBuffer[4];
    while(TRUE){
        sem_wait(&sem_columnStart);
        if(bricksLeft[colID]){
            for(i = 0; i < ROWS; i++){
                if(activeBricks[colID][i]){
                    collision = checkCollideBrick(&ball, &b);
                    dataBuffer[0] = MBOX_MSG_DRAW_BRICK;
                    dataBuffer[1] = x;
                    dataBuffer[2] = (int)(CEIL + BRICK_SPACING * (i+1) + BRICK_HEIGHT * (i + 0.5));
                    dataBuffer[3] = goldenColumn[colID] ? BRICK_COLOR_ACTIVE : BRICK_COLOR_DEFAULT;
                    XMbox_WriteBlocking(&mailbox, (u32*)dataBuffer, MBOX_MSG_DRAW_BRICK_SIZE + MBOX_MSG_ID_SIZE);
										//TODO: not check collision with all bricks (unnecessary)
										//Send collision code to primarycore
										b = toBrick(colID, i);
										collision = checkCollideBrick(&ball, &b);
										if(collision) {
                        dataBuffer[0] = MBOX_MSG_COLLISION;
                        dataBuffer[1] = checkCollideBrick(&ball, &b);
                        XMbox_WriteBlocking(&mailbox, (u32*)dataBuffer, MBOX_MSG_COLLISION_SIZE + MBOX_MSG_ID_SIZE);
										}


                }
            }
        }
        sem_post(&sem_updateComplete);
        sem_wait(&sem_columnWait);
    }
}

Brick toBrick(int colID, int bricknum) {
    Brick b;
    b.x = LEFT_WALL + BRICK_SPACING * (colID + 1) + BRICK_WIDTH * (colID + 0.5);
    b.y = (int) (CEIL + BRICK_SPACING * (bricknum+1) + BRICK_HEIGHT * (bricknum + 0.5));
    return b;
}

void* thread_col0(void){
    columnCode(0);
    return 0;
}
void* thread_col1(void){
    columnCode(1);
    return 0;
}
void* thread_col2(void){
    columnCode(2);
    return 0;
}
void* thread_col3(void){
    columnCode(3);
    return 0;
}
void* thread_col4(void){
    columnCode(4);
    return 0;
}
void* thread_col5(void){
    columnCode(5);
    return 0;
}
void* thread_col6(void){
    columnCode(6);
    return 0;
}
void* thread_col7(void){
    columnCode(7);
    return 0;
}
void* thread_col8(void){
    columnCode(8);
    return 0;
}
void* thread_col9(void){
    columnCode(9);
    return 0;
}
