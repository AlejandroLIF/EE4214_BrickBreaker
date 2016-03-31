#ifndef XMUTEXCONFIG_H
#define XMUTEXCONFIG_H

#include "xmutex.h"
#include "xparameters.h"

// declare the Hardware Mutex
#define MUTEX_DEVICE_ID XPAR_MUTEX_0_IF_1_DEVICE_ID
#define MUTEX_NUM 0

#define XST_SUCCESS 0L
#define XST_FAILURE 1L

static XMutex Mutex;

int initXMutex();
void safePrint(const char *ptr);
#endif
