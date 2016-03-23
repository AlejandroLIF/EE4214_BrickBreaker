#include "primarycore.h"

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
    /*BEGIN MAILBOX INITIALIZATION*/
	XMbox_Config *ConfigPtr;
	ConfigPtr = XMbox_LookupConfig(MBOX_DEVICE_ID);
	if(ConfigPtr == (XMbox_Config *)NULL){
		print("Error configuring mailbox uB1 Receiver\r\n");
		return XST_FAILURE;
	}

	int status;
	status = XMbox_CfgInitialize(&mailbox, ConfigPtr, ConfigPtr->BaseAddress);
	if (status != XST_SUCCESS) {
		print("Error initializing mailbox uB1 Receiver--\r\n");
		return XST_FAILURE;
	}

    //TODO: config TFT controller

    pthread_attr_init(&attr);
	schedpar.sched_priority = PRIO_HIGHEST;
	pthread_attr_setschedparam(&attr,&schedpar);
    pthread_create(&pthread_mainLoop, &attr, (void*)thread_mainLoop, NULL);

    schedpar.sched_priority--; //Lower the priority of future threads
    pthread_attr_setschedparam(&attr, &schedpar);
    pthread_create(&pthread_drawGameArea, &attr, (void*)thread_drawGameArea, NULL);
    pthread_create(&pthread_brickCollisionListener, &attr, (void*)thread_brickCollisionListener, NULL);
    pthread_create(&pthread_brickUpdateCompleteListener, &attr, (void*)thread_brickUpdateCompleteListener, NULL);
    pthread_create(&pthread_drawStatusArea, &attr, (void*)thread_drawStatusArea, NULL);
}

//Game mainloop thread
void* thread_mainLoop(void){
    while(1){
        //Welcome
        thread_welcome();
        while(!(buttonInput & BUTTON_CENTER));

        //Running
        while(!dead && !win){
            //Ready
            while(!launch){
                thread_ready();
            }

            //Running
            while(!win && !loseLife){
                if(!paused){
                    thread_running();
                }
                else{
                    //Paused
                    thread_paused();
                }
            }

            if(loseLife){
                //Lost Life
                loseLife = 0;
                thread_loseLife();
            }
        }
        if(dead){
            //Game Over
            thread_gameOver();
        }
        else{
            //Win
            thread_win();
        }
        //Wait for keypress to restart game
        while(!(buttonInput & BUTTON_CENTER));
        //FIXME: we need a thread that un-initializes all semaphores and possibly other system variables that are initialized in thread_welcome
    }
}

void* thread_welcome(void){
    if(sem_init(&sem_running, SEM_SHARED, SEM_BLOCKED)){
        print("Error while initializing semaphore sem_running!\r\n");
        while(1); //Trap runtime here
    }
    if(sem_init(&sem_drawGameArea, SEM_SHARED, SEM_AVAILABLE)){
        print("Error while initializing semaphore sem_drawGameArea!\r\n");
        while(1); //Trap runtime here
    }
    if(sem_init(&sem_brickCollisionListener, SEM_SHARED, SEM_BLOCKED)){
        print("Error while initializing semaphore sem_brickCollisionListener!\r\n");
        while(1); //Trap runtime here
    }
    if(sem_init(&sem_brickUpdateCompleteListener, SEM_SHARED, SEM_BLOCKED)){
        print("Error while initializing semaphore sem_brickUpdateCompleteListener!\r\n");
        while(1); //Trap runtime here
    }

    int restartMessage[MBOX_DATA_SIZE] = {MBOX_SIGNAL_RESTART, MBOX_SIGNAL_RESTART};
    //Send a message to the secondary core, signaling a restart
    //The secondary core should reply with a draw message for every brick
    XMbox_SendBlocking(&mailbox, (u32*) restartMessage, MBOX_DATA_SIZE); //FIXME: potential bug in data size found. FIX: send a ball whose fields are all MBOX_SIGNAL_RESTART?

    //TODO: reset paddle position
    bar = Bar_default;
    //TODO: send message to draw paddle
    //TODO: reset ball position
    ball = Ball_default;
    //TODO: send message to draw ball

    //TODO: receive draw messages
    //TODO: draw received messages (including the bricks sent by the secondary core)

    drawStatusArea();
    //TODO: draw welcome text
}

void* thread_ready(void){
    switch(barMovementCode){
        case BAR_NO_MOVEMENT:
            break; //Do nothing
        case BAR_MOVE_LEFT:
            moveLeft(&bar);
            moveLeft(&ball);
            break;
        case BAR_MOVE_RIGHT:
            moveRight(&bar);
            moveRight(&ball);
            break;
        case BAR_JUMP_LEFT:
            jumpLeft(&bar);
            jumpLeft(&ball);
            break;
        case BAR_JUMP_RIGHT:
            jumpRight(&bar);
            jumpRight(&ball);
            break;
        default:
            while(1);   //THIS SHOULD NEVER HAPPEN
    }
    queueDraw(MSG_TYPE_BAR, &bar, BAR_SIZE);
    queueDraw(MSG_TYPE_BALL, &ball, BALL_SIZE);
    //TODO: sleep(SLEEPCONSTANT);
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

void* thread_running(void){
    updateBar();
    updateBall();
    XMbox_SendBlocking(&mailbox, (u32*) &ball, BALL_SIZE);
    //Receive brick information and draw everything on screen.
    sem_post(&sem_drawGameArea);
    sem_post(&sem_brickCollisionListener);
    sem_post(&sem_brickUpdateCompleteListener);
    //Wait for the three branched threads to finish, regardless of the order.
    sem_wait(&sem_running);
    sem_wait(&sem_running);
    sem_wait(&sem_running);
    //Draw the status area
    sem_post(&sem_drawStatusArea);
    //Wait for the drawing operation to complete.
    sem_wait(&sem_running);
}

//Receives messagequeue messages
void* thread_drawGameArea(void){
    while(1){
        sem_wait(&sem_drawGameArea);
        //TODO: draw background ("clean" the frame)
        while(!brickUpdateComplete){ //FIXME: while !brickUpdateComplete or there are still elements waiting to be drawn
            //TODO: check the msgqueue for elements which require drawing and draw them
        }
        sem_post(&sem_running); //Signal the running thread that we're done. FIXME: verify thread to be signaled
    }
}

//Receives messagequeue messages
void* thread_brickCollisionListener(void){
    while(1){
        sem_wait(&sem_brickCollisionListener);
        while(!brickUpdateComplete){ //FIXME: while !brickUpdateComplete or there are still elements waiting to be drawn
            //TODO: check the msgqueue for brick collisions and update ball and score
        }
        sem_post(&sem_running); //Signal the running thread that we're done. FIXME: verify thread to be signaled
    }
}


//Receives mailbox messages from the secondary core
void* thread_brickUpdateCompleteListener(void){
    while(1){
        sem_wait(&sem_brickUpdateCompleteListener);
        brickUpdateComplete = 0;
        while(!brickUpdateComplete){
            //TODO: read incoming packages
            //TODO: if incoming package is brickUpdateComplete, set brickUpdateComplete to 1
        }
        sem_post(&sem_running); //Signal the running thread that we're done. FIXME: verify thread to be signaled
    }

}

void* thread_drawStatusArea(void){
    while(1){
        sem_wait(&sem_drawStatusArea);
        //TODO: draw background ("clean" the frame)
        //TODO: draw the status area
        sem_post(&sem_running); //Signal the running thread that we're done. FIXME: verify thread to be signaled
    }
}
