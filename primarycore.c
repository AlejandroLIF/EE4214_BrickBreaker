#include "primarycore.h"

XGpio gpPB; //PB device instance.

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
        default: //No movement if more than one button is pressed at a time.
            barMovementCode = BAR_NO_MOVEMENT;
    }
    if(buttonInput & BUTTON_CENTER){
        paused = !paused;
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
		xil_printf("ERROR initializing XGpio: Device not found");
	}
	// set PB gpio direction to input.
	XGpio_SetDataDirection(&gpPB, 1, 0x000000FF);
	//global enable
	XGpio_InterruptGlobalEnable(&gpPB);
	// interrupt enable. both global enable and this function should be called to enable gpio interrupts.
	XGpio_InterruptEnable(&gpPB,1);
	//register the handler with xilkernel
	register_int_handler(XPAR_MICROBLAZE_0_AXI_INTC_AXI_GPIO_0_IP2INTC_IRPT_INTR, gpPBIntHandler, &gpPB);
	//enable the interrupt in xilkernel
	enable_interrupt(XPAR_MICROBLAZE_0_AXI_INTC_AXI_GPIO_0_IP2INTC_IRPT_INTR);
    /*END INTERRUPT CONFIGURATION*/

    /*BEGIN TFT CONTROLLER INITIALIZATION*/
    status = TftInit(TFT_DEVICE_ID);
	if ( status != XST_SUCCESS) {
		return XST_FAILURE;
	}
    /*END TFT CONTROLLER INITIALIZATION*/

    //Initialize thread semaphores
    if(sem_init(&sem_running, SEM_SHARED, SEM_BLOCKED)){
        print("Error while initializing semaphore sem_running!\r\n");
        while(TRUE); //Trap runtime here
    }
    if(sem_init(&sem_drawGameArea, SEM_SHARED, SEM_BLOCKED)){
        print("Error while initializing semaphore sem_drawGameArea!\r\n");
        while(TRUE); //Trap runtime here
    }
    if(sem_init(&sem_brickCollisionListener, SEM_SHARED, SEM_BLOCKED)){
        print("Error while initializing semaphore sem_brickCollisionListener!\r\n");
        while(TRUE); //Trap runtime here
    }
    if(sem_init(&sem_mailboxListener, SEM_SHARED, SEM_BLOCKED)){
        print("Error while initializing semaphore sem_mailboxListener!\r\n");
        while(TRUE); //Trap runtime here
    }
    if(sem_init(&sem_drawStatusArea, SEM_SHARED, SEM_BLOCKED)){
        print("Error while initializing semaphore sem_drawStatusArea!\r\n");
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
    pthread_create(&pthread_mailboxListener, &attr, (void*)thread_mailboxListener, NULL);

    schedpar.sched_priority++;
    pthread_attr_setschedparam(&attr, &schedpar);
    pthread_create(&pthread_brickCollisionListener, &attr, (void*)thread_brickCollisionListener, NULL);

    schedpar.sched_priority++;
    pthread_attr_setschedparam(&attr, &schedpar);
    pthread_create(&pthread_drawGameArea, &attr, (void*)thread_drawGameArea, NULL);

    schedpar.sched_priority++;
    pthread_attr_setschedparam(&attr, &schedpar);
    pthread_create(&pthread_drawStatusArea, &attr, (void*)thread_drawStatusArea, NULL);
}

//Game mainloop thread
void* thread_mainLoop(void){
    while(TRUE){
        //Welcome
        welcome();
        while(!(buttonInput & BUTTON_CENTER));//while(!start)

        //Running
        while(lives && !win){
            //Ready
            while(!(buttonInput & BUTTON_CENTER)){ //while(!launch)
                ready();
                //TODO: sleep
            }

            //Running
            while(!win && !loseLife){
                if(!paused){
                    running();
                }
                else{
                    //Paused
                    paused();
                }
                //TODO: sleep
            }

            //Lost Life
            if(loseLife){
                loseLife = FALSE;
                lives--;
            }
        }
        if(!lives){
            //Game Over
            gameOver();
        }
        else{
            //Win
            win();
        }
        //Wait for keypress to restart game
        while(!(buttonInput & BUTTON_CENTER)); //while(!restart)
    }
}

void* welcome(void){
    lives = INITIAL_LIVES;
    bar = Bar_default;
    queueDraw(MSG_TYPE_BAR, &bar, BAR_SIZE);
    ball = Ball_default;
    queueDraw(MSG_TYPE_BALL, &ball, BALL_SIZE);

    //Send a message to the secondary core, signaling a restart
    //The secondary core should reply with a draw message for every brick
    MBOX_MSG_TYPE restartMessage = MBOX_MSG_RESTART;
    XMbox_SendBlocking(&mailbox, &restartMessage, 1);

    //Receive brick information and draw everything on screen.
    sem_post(&sem_drawGameArea);
    sem_post(&sem_mailboxListener);
    sem_post(&sem_brickCollisionListener);
    //Wait for the three branched threads to finish, regardless of the order.
    sem_wait(&sem_running);
    sem_wait(&sem_running);
    sem_wait(&sem_running);

    //Draw the status area
    sem_post(&sem_drawStatusArea);
    //Wait for the drawing operation to complete.
    sem_wait(&sem_running);
    //TODO: draw welcome text
}

void* ready(void){
    updateBar(&bar, barMovementCode);
    followBar(&ball, &bar);
    //FIXME: clear previous bar and ball before redrawing.
    queueDraw(MSG_TYPE_BAR, &bar, BAR_SIZE);
    queueDraw(MSG_TYPE_BALL, &ball, BALL_SIZE);
}

void queueDraw(const MSG_TYPE msgType, void* data, int size){
    int msgid;
    msgid = msgget(msgType, IPC_CREAT);

    if( msgid == -1 ) {
		xil_printf ("Error while queueing draw data. MSG_TYPE:%d\tsize:%d\tErrno: %d\r\n", msgType, size, errno);
		pthread_exit (&errno);
	}
	if(msgsnd(msgid, data, size, 0) < 0 ) { // blocking send
        xil_printf ("Msgsnd of message(%d) ran into ERROR. Errno: %d. Halting..\r\n", msgType, errno);
		pthread_exit(&errno);
	}
}

void* running(void){
    updateBar(&bar, barMovementCode);
    updateBall(&ball);
    unsigned int message[6];
    buildBallMessage(&ball, message);
    //Send the ball position to the secondary core to initialize collision checking
    XMbox_SendBlocking(&mailbox, (u32*) message, BALL_SIZE + 1);

    //Receive brick information and draw everything on screen.
    sem_post(&sem_drawGameArea);
    sem_post(&sem_brickCollisionListener);
    sem_post(&sem_mailboxListener);
    //Wait for the three branched threads to finish, regardless of the order.
    sem_wait(&sem_running);
    sem_wait(&sem_running);
    sem_wait(&sem_running);

    //Draw the status area
    sem_post(&sem_drawStatusArea);
    //Wait for the drawing operation to complete.
    sem_wait(&sem_running);
}

void buildBallMessage(Ball* ball, unsigned int message){
    message[0] = MBOX_MSG_BALL;
    message[1] = ball->x;
    message[2] = ball->y;
    message[3] = ball->d;
    message[4] = ball->s;
    message[5] = ball->c;
}

//Receives messagequeue messages
void* thread_drawGameArea(void){
    int hasDrawn;
    while(TRUE){
        hasDrawn = TRUE;
        sem_wait(&sem_drawGameArea);
        //TODO: draw background ("clean" the frame)
        while(!brickUpdateComplete || hasDrawn){
            hasDrawn = FALSE;
            //TODO: if draw, hasDrawn = 1;
            //TODO: check the msgqueue for elements which require drawing and draw them
        }
        sem_post(&sem_running); //Signal the running thread that we're done. FIXME: verify thread to be signaled
    }
}

//Receives messagequeue messages
void* thread_brickCollisionListener(void){
    int hasCollided;
    while(TRUE){
        hasCollided = FALSE;
        sem_wait(&sem_brickCollisionListener);
        while(!brickUpdateComplete || hasCollided){
            //TODO: if collision occurs, hasCollided = 1
            //TODO: check the msgqueue for brick collisions and update ball and score
        }
        sem_post(&sem_running); //Signal the running thread that we're done. FIXME: verify thread to be signaled
    }
}


//Receives mailbox messages from the secondary core
void* thread_mailboxListener(void){
    Brick brick;
    MBOX_MSG_TYPE msgType;
    while(TRUE){
        sem_wait(&sem_mailboxListener);
        brickUpdateComplete = FALSE;
        while(!brickUpdateComplete){
            XMbox_ReadBlocking(&mailbox, (u32*)&msgType, 1);
            switch(msgType){
                case MBOX_MSG_DRAW_BRICK:
                    //TODO
                    break;
                case MBOX_MSG_COLLISION:
                    //TODO
                    break;
                case MBOX_MSG_END_COMPUTATION:
                    brickUpdateComplete = TRUE;
                    break;
                case default:
                    while(TRUE); //This should not happen. Trap runtime!
            }
        }
        sem_post(&sem_running); //Signal the running thread that we're done. FIXME: verify thread to be signaled
    }
}

void* thread_drawStatusArea(void){
    while(TRUE){
        sem_wait(&sem_drawStatusArea);
        //TODO: draw background ("clean" the frame)
        //TODO: draw the status area
        sem_post(&sem_running); //Signal the running thread that we're done. FIXME: verify thread to be signaled
    }
}
