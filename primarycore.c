#include "primarycore.h"

XGpio gpPB; //PB device instance.
Bar bar = {0, FLOOR - 10 - 3};
Ball ball;

unsigned int cyclesElapsed;

static void gpPBIntHandler(void *arg)
{
    //clear the interrupt flag. if this is not done, gpio will keep interrupting the microblaze.--
    // --Possible to use (XGpio*)arg instead of &gpPB
    XGpio_InterruptClear(&gpPB,1);
    //Read the state of the push buttons.
    buttonInput = XGpio_DiscreteRead(&gpPB, 1);
    //TODO: configure bar movement codes for "jump"
    switch(buttonInput){
        case BUTTON_LEFT:
        barMovementCode = BAR_MOVE_LEFT;
        break;
        case BUTTON_RIGHT:
        barMovementCode = BAR_MOVE_RIGHT;
        break;
        case BUTTON_UP:
        win = TRUE; //FIXME: THIS IS USED TO TEST SOFT-RESET FUNCTIONALITY
        break;
        default: //No movement if more than one button is pressed at a time.
        barMovementCode = BAR_NO_MOVEMENT;
    }
    if(buttonInput & BUTTON_CENTER){
        paused = !paused;
    }

    if(gameCycles - lastInterrupt < DEBOUNCE_CYCLES) return;
    if(buttonInput){
        lastInterrupt = gameCycles;
    }
    else{
        if(gameCycles - lastInterrupt < JUMP_CYCLE_THRESHOLD){
            if(barMovementCode == BAR_MOVE_LEFT){
                barMovementCode = BAR_JUMP_LEFT;
            }
            else if(barMovementCode == BAR_MOVE_RIGHT){
                barMovementCode = BAR_JUMP_RIGHT;
            }
        }
    }
}

//Firmware entry point
int main(void){
    xilkernel_init();
    xmk_add_static_thread(main_prog,0);
    xilkernel_start();
    xilkernel_main ();
    return 0;
}

//Xilkernel entry point
int main_prog(void){
    int status;

    /*BEGIN MAILBOX INITIALIZATION*/
    XMbox_Config *ConfigPtr;
    ConfigPtr = XMbox_LookupConfig(MBOX_DEVICE_ID);
    if(ConfigPtr == (XMbox_Config *)NULL){
        print("Error configuring mailbox uB1 Receiver\r\n");
        return XST_FAILURE;
    }

    status = XMbox_CfgInitialize(&mailbox, ConfigPtr, ConfigPtr->BaseAddress);
    if (status != XST_SUCCESS) {
        print("Error initializing mailbox uB1 Receiver--\r\n");
        return XST_FAILURE;
    }
    /*END MAILBOX INITIALIZATION*/

    /*BEGIN INTERRUPT CONFIGURATION*/
    // Initialise the PB instance
    status = XGpio_Initialize(&gpPB, XPAR_GPIO_0_DEVICE_ID);
    if (status == XST_DEVICE_NOT_FOUND) {
        safePrint("ERROR initializing XGpio: Device not found");
    }
    // set PB gpio direction to input.
    XGpio_SetDataDirection(&gpPB, 1, 0x000000FF);
    //global enable
    XGpio_InterruptGlobalEnable(&gpPB);
    // interrupt enable. both global enable and this function should be called to enable gpio interrupts.
    XGpio_InterruptEnable(&gpPB, 1);
    //register the handler with xilkernel
    register_int_handler(XPAR_MICROBLAZE_0_AXI_INTC_AXI_GPIO_0_IP2INTC_IRPT_INTR, gpPBIntHandler, &gpPB);
    //enable the interrupt in xilkernel
    enable_interrupt(XPAR_MICROBLAZE_0_AXI_INTC_AXI_GPIO_0_IP2INTC_IRPT_INTR);
    /*END INTERRUPT CONFIGURATION*/

    /*BEGIN TFT CONTROLLER INITIALIZATION*/
    status = TftInit(TFT_DEVICE_ID, &TftInstance);
    if ( status != XST_SUCCESS) {
        return XST_FAILURE;
    }
    /*END TFT CONTROLLER INITIALIZATION*/

    /* BEGIN XMUTEX INITIALIZATION */
    status = initXMutex();
    if (status != XST_SUCCESS) {
        return XST_FAILURE;
    }
    /* END XMUTEX INITIALIZATION */

    //Initialize thread semaphores
    if(sem_init(&sem_running, SEM_SHARED, SEM_BLOCKED)){
        safePrint("Error while initializing semaphore sem_running!\r\n");
        while(TRUE); //Trap runtime here
    }
    if(sem_init(&sem_drawGameArea, SEM_SHARED, SEM_BLOCKED)){
        safePrint("Error while initializing semaphore sem_drawGameArea!\r\n");
        while(TRUE); //Trap runtime here
    }
    if(sem_init(&sem_brickCollisionListener, SEM_SHARED, SEM_BLOCKED)){
        safePrint("Error while initializing semaphore sem_brickCollisionListener!\r\n");
        while(TRUE); //Trap runtime here
    }
    if(sem_init(&sem_mailboxListener, SEM_SHARED, SEM_BLOCKED)){
        safePrint("Error while initializing semaphore sem_mailboxListener!\r\n");
        while(TRUE); //Trap runtime here
    }
    if(sem_init(&sem_drawStatusArea, SEM_SHARED, SEM_BLOCKED)){
        safePrint("Error while initializing semaphore sem_drawStatusArea!\r\n");
        while(TRUE); //Trap runtime here
    }
    /*
    Thread priority (0 is highest priority):
    1. thread_mainLoop:
    • highest priority: preempts all other threads while not blocked (waiting for them to finish)
    2. thread_mailboxListener
    • If the mailbox is blocked, the second core also stalls.
    3. thread_brickCollisionListener
    • Update appropriate game values if the ball collided with a brick.
    4. thread_drawGameArea
    • FIXME: Possible issue: Drawing the game area is time-consuming and will require messagequeue usage. Messagequeue overflow?
    5. thread_drawStatusArea
    • Low-priority thread. After it is run, the frame is considered ready to be displayed.
    */
    pthread_attr_init(&attr);
    schedpar.sched_priority = PRIO_HIGHEST;
    pthread_attr_setschedparam(&attr,&schedpar);
    pthread_create(&pthread_mainLoop, &attr, (void*)thread_mainLoop, NULL);

    schedpar.sched_priority++;
    pthread_attr_setschedparam(&attr, &schedpar);
    pthread_create(&pthread_brickCollisionListener, &attr, (void*)thread_brickCollisionListener, NULL);

    schedpar.sched_priority++;
    pthread_attr_setschedparam(&attr, &schedpar);
    pthread_create(&pthread_drawGameArea, &attr, (void*)thread_drawGameArea, NULL);

    schedpar.sched_priority++;
    pthread_attr_setschedparam(&attr, &schedpar);
    pthread_create(&pthread_drawStatusArea, &attr, (void*)thread_drawStatusArea, NULL);

    schedpar.sched_priority++;
    pthread_attr_setschedparam(&attr, &schedpar);
    pthread_create(&pthread_mailboxListener, &attr, (void*)thread_mailboxListener, NULL);

    return 0;
}

//Game mainloop thread
void* thread_mainLoop(void){
    while(TRUE){
        //Welcome
        //safePrint("primarycore: Welcome\r\n");
        welcome();
        while(!(buttonInput & BUTTON_CENTER));//while(!start)
        sleep(1000); //FIXME: hardcoded delay
        //Running
        while(lives && !win){
            //Ready
            while(FALSE/*!(buttonInput & BUTTON_CENTER)*/){ //while(!launch) //FIXME: QUEUES MESSAGES, BUT NOTHING IS READING THEM
                // safePrint("primarycore: ready\r\n");
                ready();
                //                sleep(SLEEPCONSTANT); //FIXME: sleep calibration
            }

            //Running
            while(!win && !loseLife){
                if(!paused){
                    safePrint("pc: run\r\n");
                    running();
                }
            }

            //Lost Life
            if(loseLife){
                loseLife = FALSE;
                lives--;
                resetBallAndBar();
                paused = TRUE; //FIXME: this allows the player to recover from losing a ball. Player should actually jump to "running"
            }
        }
        if(!lives){
            //Game Over
            gameOver();
        }
        else{
            //Win
            gameWin();
        }
        //Wait for keypress to restart game
        while(!(buttonInput & BUTTON_CENTER)); //while(!restart)
    }
}

void welcome(void){
    int drawGameAreaBackground = 1;
    unsigned int dataBuffer[5];
    lives = INITIAL_LIVES;
    //FIXME: hacky fix with the bar.x1
    queueMsg(MSGQ_TYPE_GAMEAREA, &drawGameAreaBackground, MSGQ_MSGSIZE_GAMEAREA);
    queueMsg(MSGQ_TYPE_STATUSAREA, &drawGameAreaBackground, MSGQ_MSGSIZE_STATUSAREA);
    nextScoreMilestone = SCORE_MILESTONE;
    scoreMilestoneReached = 0;
    score = 0;
    win = FALSE;
    gameCycles = 0;
    lastInterrupt = 0;

    resetBallAndBar();

    //Send a message to the secondary core, signaling a restart
    //The secondary core should reply with a draw message for every brick
    MBOX_MSG_TYPE restartMessage = MBOX_MSG_RESTART;
    dataBuffer[0] = MBOX_MSG_BEGIN_COMPUTATION;
    dataBuffer[1] = ball.x;
    dataBuffer[2] = ball.y;
    XMbox_WriteBlocking(&mailbox, (u32*)&restartMessage, MBOX_MSG_ID_SIZE);
    XMbox_WriteBlocking(&mailbox, (u32*)dataBuffer, MBOX_MSG_BEGIN_COMPUTATION_SIZE);
    //Receive brick information and draw everything on screen.
    sem_post(&sem_mailboxListener);
    //Wait for the three branched threads to finish, regardless of the order.
    sem_wait(&sem_running);
    //If a score milestone was reached, update ball speed and golden columns
    while(scoreMilestoneReached){
        updateBallSpeed(&ball, BALL_SPEED_SCORE_INCREASE);              //Update the ball's speed
        dataBuffer[0] = MBOX_MSG_UPDATE_GOLDEN;                         //Update golden bricks.
        XMbox_WriteBlocking(&mailbox, (u32*) dataBuffer, MBOX_MSG_UPDATE_GOLDEN_SIZE);
        XMbox_WriteBlocking(&mailbox, (u32*) dataBuffer, MBOX_MSG_UPDATE_GOLDEN_SIZE);
        scoreMilestoneReached--;    //Do not decrease scoreMilestoneReached in the loop condition evaluation, as it should not be decreased unless it's positive.
    }

    //Only draw the status area if lives or score has been updated
    if(last_drawn_lives != lives || last_drawn_score != score) {
        sem_post(&sem_drawStatusArea);
        last_drawn_lives = lives;
        last_drawn_score = score;
        //Wait for the drawing operation to complete.
        sem_wait(&sem_running);
    }

    //TODO: draw welcome text
}

void ready(void){
	unsigned int dataBuffer[3];
    //Erase the bar
    bar.c = GAMEAREA_COLOR;
    dataBuffer[0] = bar.x;
    dataBuffer[1] = bar.y;
    dataBuffer[2] = bar.c;
    queueMsg(MSGQ_TYPE_BAR, dataBuffer, MSGQ_MSGSIZE_BAR);
    bar.c = COLOR_NONE;
    //Erase the ball
    ball.c = GAMEAREA_COLOR;
    dataBuffer[0] = ball.x;
    dataBuffer[1] = ball.y;
    dataBuffer[2] = ball.c;
    queueMsg(MSGQ_TYPE_BALL, dataBuffer, MSGQ_MSGSIZE_BALL);

    updateBar(&bar, barMovementCode);
    //FIXME: hacky implementation of "jump" functionality requires manual reset
    if(barMovementCode == BAR_JUMP_LEFT || barMovementCode == BAR_JUMP_RIGHT){
        barMovementCode = BAR_NO_MOVEMENT;
    }
    followBar(&ball, &bar);
    dataBuffer[0] = bar.x;
    dataBuffer[1] = bar.y;
    dataBuffer[2] = bar.c;
    queueMsg(MSGQ_TYPE_BAR, dataBuffer, MSGQ_MSGSIZE_BAR);
    dataBuffer[0] = ball.x;
    dataBuffer[1] = ball.y;
    dataBuffer[2] = ball.c;
    queueMsg(MSGQ_TYPE_BALL, dataBuffer, MSGQ_MSGSIZE_BALL);
}

void resetBallAndBar(void){
	unsigned int dataBuffer[3];
	bar.x = (LEFT_WALL + RIGHT_WALL)/2;
	queueMsg(MSGQ_TYPE_BAR, &bar, MSGQ_MSGSIZE_BAR);
	// ball = Ball_default; //FIXME: restore Ball_default
	ball.x = bar.x;
	ball.y = BAR_Y - DIAMETER / 2;
	ball.d = 340;
	ball.s = 5;
	ball.c = 0;
	dataBuffer[0] = ball.x;
	dataBuffer[1] = ball.y;
	dataBuffer[2] = ball.c;
	queueMsg(MSGQ_TYPE_BALL, dataBuffer, MSGQ_MSGSIZE_BALL);
}

void queueMsg(const MSGQ_TYPE msgType, void* data, const MSGQ_MSGSIZE size){
    int msgid;
    msgid = msgget(msgType, IPC_CREAT);
    if( msgid == -1 ) {
        xil_printf ("Error while queueing. MSG_TYPE:%d\tsize:%d\tErrno: %d\r\n", msgType, size, errno);
        pthread_exit (&errno);
    }
    if(msgsnd(msgid, data, size, 0) < 0 ) { // blocking send
        xil_printf ("Msgsnd of message(%d) ran into ERROR. Errno: %d. Halting..\r\n", msgType, errno);
        pthread_exit(&errno);
    }
}

void running(void){
    ticks_before = xget_clock_ticks();
    int drawGameAreaBackground = 1;
    int dataBuffer[5];
    //Erase the bar
    bar.c = GAMEAREA_COLOR;
    dataBuffer[0] = bar.x;
    dataBuffer[1] = bar.y;
    dataBuffer[2] = bar.c;
    queueMsg(MSGQ_TYPE_BAR, dataBuffer, MSGQ_MSGSIZE_BAR);
    bar.c = COLOR_NONE;
    //Erase the ball
    ball.c = GAMEAREA_COLOR;
    dataBuffer[0] = ball.x;
    dataBuffer[1] = ball.y;
    dataBuffer[2] = ball.c;
    queueMsg(MSGQ_TYPE_BALL, dataBuffer, MSGQ_MSGSIZE_BALL);
    ball.c = COLOR_NONE;
    sem_post(&sem_drawGameArea);

    updateBar(&bar, barMovementCode);
    //FIXME: hacky implementation of "jump" functionality requires manual reset
    if(barMovementCode == BAR_JUMP_LEFT || barMovementCode == BAR_JUMP_RIGHT){
        barMovementCode = BAR_NO_MOVEMENT;
    }

    updateBallPosition(&ball);
    dataBuffer[0] = bar.x;
    dataBuffer[1] = bar.y;
    dataBuffer[2] = bar.c;
    queueMsg(MSGQ_TYPE_BAR, dataBuffer, MSGQ_MSGSIZE_BAR);
    dataBuffer[0] = ball.x;
    dataBuffer[1] = ball.y;
    dataBuffer[2] = ball.c;
    queueMsg(MSGQ_TYPE_BALL, dataBuffer, MSGQ_MSGSIZE_BALL);

    //Check collision with walls
    updateBallDirection(&ball, checkCollideWall(&ball));

    //Check collision with bar
    updateBallDirection(&ball, checkCollideBar(&ball, &bar));
    dataBuffer[0] = MBOX_MSG_BEGIN_COMPUTATION;
    dataBuffer[1] = ball.x;
    dataBuffer[2] = ball.y;
    //Send the ball position to the secondary core to initialize collision checking
    XMbox_WriteBlocking(&mailbox, (u32*) dataBuffer, MBOX_MSG_BEGIN_COMPUTATION_SIZE);
    hasCollided = FALSE;
    //Receive brick information and draw everything on screen.
    sem_post(&sem_mailboxListener);
    //Wait for the three branched threads to finish
    sem_wait(&sem_running);
    //If a score milestone was reached, update ball speed and golden columns
    while(scoreMilestoneReached){
        updateBallSpeed(&ball, BALL_SPEED_SCORE_INCREASE);              //Update the ball's speed
        dataBuffer[0] = MBOX_MSG_UPDATE_GOLDEN;                         //Update golden bricks.
        XMbox_WriteBlocking(&mailbox, (u32*) dataBuffer, MBOX_MSG_UPDATE_GOLDEN_SIZE);
        XMbox_WriteBlocking(&mailbox, (u32*) dataBuffer, MBOX_MSG_UPDATE_GOLDEN_SIZE);
        scoreMilestoneReached--;    //Do not decrease scoreMilestoneReached in the loop condition evaluation, as it should not be decreased unless it's positive.
    }

    //Only draw the status area if lives or score has been updated
    if(last_drawn_lives != lives || last_drawn_score != score || last_drawn_speed != ball.s) {
        sem_post(&sem_drawStatusArea);
        last_drawn_lives = lives;
        last_drawn_score = score;
        last_drawn_speed = ball.s;
        //Wait for the drawing operation to complete.
        sem_wait(&sem_running);
    }

    //Lock framerate to specified FPS
    ticks_diff = xget_clock_ticks();
    if(ticks_diff > ticks_before){
        ticks_diff -= ticks_before;
    }
    else{
        ticks_diff += 0xFFFFFFFF - ticks_before; //In case of overflow
    }

    if(ticks_diff < PERIOD_TICKS){				//If we have time to spare
        sleep(PERIOD_TICKS - ticks_diff);
    }
    gameCycles++;
}

//Receives messagequeue messages
void* thread_drawGameArea(void){
    unsigned int dataBuffer[3];
    while(TRUE){
        sem_wait(&sem_drawGameArea); //Wait to be signaled
        // while(readFromMessageQueue(MSGQ_TYPE_BACKGROUND, dataBuffer, MSGQ_MSGSIZE_BACKGROUND)){
        //     safePrint("primarycore: drawBackground\r\n");
        //     draw(dataBuffer, MSGQ_TYPE_BACKGROUND);
        // }
        while(readFromMessageQueue(MSGQ_TYPE_GAMEAREA, dataBuffer, MSGQ_MSGSIZE_GAMEAREA)){
            // safePrint("primarycore: drawBackground\r\n");
            draw(dataBuffer, MSGQ_TYPE_GAMEAREA);
            safePrint("pc: dbg\r\n");
        }
        while(readFromMessageQueue(MSGQ_TYPE_BAR, dataBuffer, MSGQ_MSGSIZE_BAR)){
            // safePrint("primarycore: drawBar\r\n");
            draw(dataBuffer, MSGQ_TYPE_BAR);
            safePrint("pc: dbar\r\n");
        }
        while(readFromMessageQueue(MSGQ_TYPE_BALL, dataBuffer, MSGQ_MSGSIZE_BALL)){
            // safePrint("primarycore: drawBall\r\n");
            draw(dataBuffer, MSGQ_TYPE_BALL);
            safePrint("pc: dball\r\n");
        }
        while(readFromMessageQueue(MSGQ_TYPE_BRICK, dataBuffer, MSGQ_MSGSIZE_BRICK)){
            // safePrint("primarycore: drawBrick\r\n");
            draw(dataBuffer, MSGQ_TYPE_BRICK);
            safePrint("pc: dbrick\r\n");
        }
    }
}

int readFromMessageQueue(const MSGQ_TYPE id, void* dataBuffer, const MSGQ_MSGSIZE size){
    int msgid = msgget (id, IPC_CREAT);
    int msgSize;
    if( msgid == -1 ) {
        xil_printf ("receiveMessage -- ERROR while opening Message Queue. Errno: %d \r\n", errno);
        pthread_exit(&errno) ;
    }
    msgSize = msgrcv(msgid, dataBuffer, size, 0, IPC_NOWAIT);
    if(msgSize != size) { // NON-BLOCKING receive
        if(errno == EAGAIN){return FALSE;}
        xil_printf ("receiveMessage of message(%d) ran into ERROR. Errno: %d. Halting...\r\n", id, errno);
        pthread_exit(&errno);
    }
    return TRUE;
}

//Receives messagequeue messages
void* thread_brickCollisionListener(void){
    unsigned int dataBuffer[3];
    unsigned char temp;
    while(TRUE){
        sem_wait(&sem_brickCollisionListener);
        //TODO: remove the while loop. no point in it :P
        while(readFromMessageQueue(MSGQ_TYPE_BRICK_COLLISION, dataBuffer, MSGQ_MSGSIZE_BRICK_COLLISION)){
            if(increaseScore(dataBuffer[1])){ //FIXME: magic numbers when interpreting the data buffer
                scoreMilestoneReached++;
            }
            //            safePrint("Brick collision!\r\n");
            //            safePrint(dataBuffer[0] + '0');
            //            if(!hasCollided){
            updateBallDirection(&ball, dataBuffer[0]); //TODO: implement method. dataBuffer[0] should be a CollisionCodeType
            hasCollided = TRUE;
            safePrint("pc: collide ");
            temp = dataBuffer[0] + '0';
            safePrint(&temp);
            safePrint("\r\n");
            //            }
        }
        // sem_post(&sem_running); //Signal the running thread that we're done.
    }
}

int increaseScore(int isGoldenBrick){
    if(isGoldenBrick){
        score += BRICK_SCORE_GOLDEN;
    }
    else{
        score += BRICK_SCORE_NORMAL;
    }

    if(score >= nextScoreMilestone){
        nextScoreMilestone += SCORE_MILESTONE;
        return TRUE;
    }
    return FALSE;
}

//Receives mailbox messages from the secondary core
void* thread_mailboxListener(void){
    MBOX_MSG_TYPE msgType;
    unsigned int dataBuffer[3]; //FIXME: magic numbers when declaring array size
    while(TRUE){
        sem_wait(&sem_mailboxListener);
        brickUpdateComplete = FALSE;
        while(!brickUpdateComplete){
            XMbox_ReadBlocking(&mailbox, (u32*)&msgType, MBOX_MSG_ID_SIZE);
            switch(msgType){
                case MBOX_MSG_DRAW_BRICK:
                XMbox_ReadBlocking(&mailbox, (u32*)dataBuffer, MBOX_MSG_DRAW_BRICK_SIZE);
                queueMsg(MSGQ_TYPE_BRICK, dataBuffer, MSGQ_MSGSIZE_BRICK);
                sem_post(&sem_drawGameArea); //TODO: fix semaphore when drawGameArea has been split into several methods
                break;
                case MBOX_MSG_COLLISION:
                XMbox_ReadBlocking(&mailbox, (u32*)dataBuffer, MBOX_MSG_COLLISION_SIZE);
                queueMsg(MSGQ_TYPE_BRICK_COLLISION, dataBuffer, MSGQ_MSGSIZE_BRICK_COLLISION);
                sem_post(&sem_brickCollisionListener);
                break;
                case MBOX_MSG_COMPUTATION_COMPLETE:
                brickUpdateComplete = TRUE;
                break;
                case MBOX_MSG_VICTORY:
                brickUpdateComplete = TRUE;
                win = TRUE;
                break;
                default:
                while(TRUE){safePrint("Invalid mailbox message\r\n");} //This should not happen. Trap runtime!
            }
        }
        sem_post(&sem_running); //Signal the running thread that we're done.
    }
}

void* thread_drawStatusArea(void){
    while(TRUE){
        sem_wait(&sem_drawStatusArea);
        draw(0, MSGQ_TYPE_STATUSAREA);
        sem_post(&sem_running); //Signal the running thread that we're done.
    }
}

//FIXME: For the drawing of the bar, replace else if chain with separate for loops for each section (or other fix)
void draw(unsigned int* dataBuffer, const MSGQ_TYPE msgType){
    int i, j;
    int x, y, c;
    int hscore, dscore, uscore, hspeed, dspeed, uspeed;
    switch(msgType){
        case MSGQ_TYPE_BRICK:
        //FIXME: hardcoded indexes
        x = dataBuffer[0];
        y = dataBuffer[1];
        c = dataBuffer[2];

        for(j = y - BRICK_HEIGHT/2; j < y + BRICK_HEIGHT/2; j++) {
            for(i = x - BRICK_WIDTH/2; i < x + BRICK_WIDTH/2; i++) {
                XTft_SetPixel(&TftInstance, i, j, c);
            }
        }


        break;

        case MSGQ_TYPE_BAR:
        //FIXME: hard-coded indexes
        x = dataBuffer[0];
        y = dataBuffer[1];
        c = dataBuffer[2];

        for(j = y - BAR_HEIGHT/2; j < y + BAR_HEIGHT/2; j++) {
            for(i = 0; i < BAR_WIDTH; i++) {
                if(i < A_REGION_WIDTH) {
                    XTft_SetPixel(&TftInstance, i + x - BAR_WIDTH/2, j, c ? c : A_REGION_COLOR);
                } else if(i < A_REGION_WIDTH + S_REGION_WIDTH) {
                    XTft_SetPixel(&TftInstance, i + x - BAR_WIDTH/2, j, c ? c : S_REGION_COLOR);
                } else if(i < A_REGION_WIDTH + S_REGION_WIDTH + N_REGION_WIDTH) {
                    XTft_SetPixel(&TftInstance, i + x - BAR_WIDTH/2, j, c ? c : N_REGION_COLOR);
                } else if(i < A_REGION_WIDTH + 2*S_REGION_WIDTH + N_REGION_WIDTH) {
                    XTft_SetPixel(&TftInstance, i + x - BAR_WIDTH/2, j, c ? c : S_REGION_COLOR);
                }else{
                    XTft_SetPixel(&TftInstance, i + x - BAR_WIDTH/2, j, c ? c : A_REGION_COLOR);
                }
            }
        }

        break;

        case MSGQ_TYPE_BALL:
        //FIXME: hardcoded indexes
        x = dataBuffer[0];
        y = dataBuffer[1];
        c = dataBuffer[2];
        for(j = 0; j < DIAMETER; j++) {
            for (i = 0; i < DIAMETER; i++) {
                if(BALL_MASK[j][i] == 0xFFFFFFFF) {
                    XTft_SetPixel(&TftInstance, x - DIAMETER/2 + i, y - DIAMETER/2 + j, c ? c : BALL_COLOR);
                }
            }
        }
        break;

        case MSGQ_TYPE_BACKGROUND:
        //Fill screen with background color
        for(j = 0; j < 479; j++) {
            for(i = 0; i < 639; i++) {
                XTft_SetPixel(&TftInstance, i, j, BACKGROUND_COLOR);
            }
        }
        break;

        case MSGQ_TYPE_GAMEAREA:
        //Draw game area
        for(j = CEIL; j < FLOOR; j++) {
            for(i = LEFT_WALL; i < RIGHT_WALL; i++) {
                XTft_SetPixel(&TftInstance, i, j, GAMEAREA_COLOR);
            }
        }
        break;

        case MSGQ_TYPE_STATUSAREA:
        //Draw score area
        for(j = SCORE_CEIL; j < SCORE_FLOOR; j++) {
            for(i = STATUS_LEFT_WALL; i < STATUS_RIGHT_WALL; i++) {
                XTft_SetPixel(&TftInstance, i, j, STATUSAREA_COLOR);
            }
        }
        XTft_SetPosChar(&TftInstance, STATUS_LEFT_WALL + STATUS_TEXT_OFFSET/4, SCORE_CEIL + STATUS_TEXT_OFFSET/4);
        XTft_SetColor(&TftInstance, STATUSAREA_SCORE_COLOR, STATUSAREA_COLOR);

        //FIXME: Need correct offset between score text and score value
        hscore = score / 100;
        dscore = (score % 100) / 10;
        uscore = score % 10;

        screenWrite("Score", 5);
        XTft_SetPosChar(&TftInstance, STATUS_LEFT_WALL + STATUS_TEXT_OFFSET, SCORE_CEIL + STATUS_TEXT_OFFSET);

        XTft_Write(&TftInstance, intToChar(hscore));
        XTft_Write(&TftInstance, intToChar(dscore));
        XTft_Write(&TftInstance, intToChar(uscore));

        //Draw LIVES area
        for(j = LIVES_CEIL; j < LIVES_FLOOR; j++) {
            for(i = STATUS_LEFT_WALL; i < STATUS_RIGHT_WALL; i++) {
                XTft_SetPixel(&TftInstance, i, j, STATUSAREA_COLOR);
            }
        }

        XTft_SetPosChar(&TftInstance, STATUS_LEFT_WALL + STATUS_TEXT_OFFSET/4, LIVES_CEIL + STATUS_TEXT_OFFSET/4);
        XTft_SetColor(&TftInstance, STATUSAREA_SCORE_COLOR, STATUSAREA_COLOR);
        screenWrite("Lives", 5);

        XTft_SetPosChar(&TftInstance, STATUS_LEFT_WALL + STATUS_TEXT_OFFSET, LIVES_CEIL + STATUS_TEXT_OFFSET);
        XTft_Write(&TftInstance, intToChar(lives));

        //Draw BALL SPEED area
        for(j = BALLSPEED_CEIL; j < BALLSPEED_FLOOR; j++) {
            for(i = STATUS_LEFT_WALL; i < STATUS_RIGHT_WALL; i++) {
                XTft_SetPixel(&TftInstance, i, j, STATUSAREA_COLOR);
            }
        }

        XTft_SetPosChar(&TftInstance, STATUS_LEFT_WALL + STATUS_TEXT_OFFSET/4, BALLSPEED_CEIL + STATUS_TEXT_OFFSET/4);
        XTft_SetColor(&TftInstance, STATUSAREA_SCORE_COLOR, STATUSAREA_COLOR);
        screenWrite("Ball speed", 10);

        hspeed = ball.s / 100;
        dspeed = (ball.s % 100) / 10;
        uspeed = ball.s % 10;

        XTft_SetPosChar(&TftInstance, STATUS_LEFT_WALL + STATUS_TEXT_OFFSET, BALLSPEED_CEIL + STATUS_TEXT_OFFSET);
        XTft_Write(&TftInstance, intToChar(hspeed));
        XTft_Write(&TftInstance, intToChar(dspeed));
        XTft_Write(&TftInstance, intToChar(uspeed));


        //Draw BRICKS LEFT area
        for(j = BRICKSLEFT_CEIL; j < BRICKSLEFT_FLOOR; j++) {
            for(i = STATUS_LEFT_WALL; i < STATUS_RIGHT_WALL; i++) {
                XTft_SetPixel(&TftInstance, i, j, STATUSAREA_COLOR);
            }
        }

        XTft_SetPosChar(&TftInstance, STATUS_LEFT_WALL + STATUS_TEXT_OFFSET/4, BRICKSLEFT_CEIL + STATUS_TEXT_OFFSET/4);
        XTft_SetColor(&TftInstance, STATUSAREA_SCORE_COLOR, STATUSAREA_COLOR);
        screenWrite("Bricks left", 11);

        //TODO: Add brick counter to be able to display this
        XTft_SetPosChar(&TftInstance, STATUS_LEFT_WALL + STATUS_TEXT_OFFSET, BRICKSLEFT_CEIL + STATUS_TEXT_OFFSET);
        //XTft_Write(&TftInstance, intToChar(lives));
        screenWrite("Todo", 4);

        //Draw PLAYTIME area
        for(j = PLAYTIME_CEIL; j < PLAYTIME_FLOOR; j++) {
            for(i = STATUS_LEFT_WALL; i < STATUS_RIGHT_WALL; i++) {
                XTft_SetPixel(&TftInstance, i, j, STATUSAREA_COLOR);
            }
        }

        XTft_SetPosChar(&TftInstance, STATUS_LEFT_WALL + STATUS_TEXT_OFFSET/4, PLAYTIME_CEIL + STATUS_TEXT_OFFSET/4);
        XTft_SetColor(&TftInstance, STATUSAREA_SCORE_COLOR, STATUSAREA_COLOR);
        screenWrite("Game time", 9);

        XTft_SetPosChar(&TftInstance, STATUS_LEFT_WALL + STATUS_TEXT_OFFSET, PLAYTIME_CEIL + STATUS_TEXT_OFFSET);
        //XTft_Write(&TftInstance, intToChar(lives));
        //FIXME: the game time should be drawn at least every second.
        //FIXME: fix hard-coded FPS
        hspeed = (gameCycles/50) / 100;
		dspeed = ((gameCycles/50) % 100) / 10;
		uspeed = (gameCycles/50) % 10;
		XTft_Write(&TftInstance, intToChar(hspeed));
		XTft_Write(&TftInstance, intToChar(dspeed));
		XTft_Write(&TftInstance, intToChar(uspeed));

        break;

        default:
        while(TRUE); //Should never happen. Trap runtime here.
    }
}

//GameOver method should display "Game Over" text and prompt the user to press a key to restart
void gameOver(void){
    safePrint("Game over!\r\n");
    XTft_SetPosChar(&TftInstance, LEFT_WALL + 150, (CEIL + FLOOR)/2);
    XTft_SetColor(&TftInstance, STATUSAREA_SCORE_COLOR, GAMEAREA_COLOR);
    screenWrite("Game Over", 9);
    XTft_SetPosChar(&TftInstance, LEFT_WALL + 100, (CEIL + FLOOR)/2 + 20);
    screenWrite("Press the middle button to restart", 34);
}

//Win method should display "Win" text and prompt the user to press a key to restart
void gameWin(void){
    safePrint("Victory!\r\n");
    XTft_SetPosChar(&TftInstance, LEFT_WALL + 150, (CEIL + FLOOR)/2);
    XTft_SetColor(&TftInstance, STATUSAREA_SCORE_COLOR, GAMEAREA_COLOR);
    screenWrite("Victory!", 8);
    XTft_SetPosChar(&TftInstance, LEFT_WALL + 100, (CEIL + FLOOR)/2 + 20);
    screenWrite("Press the middle button to restart", 34);
}

unsigned char intToChar(int n){
    return (unsigned char)(n + 48);
}

void screenWrite(char* str, int size) {
    int i;
    for(i = 0; i < size; i++) {
        XTft_Write(&TftInstance, str[i]);
    }
}
