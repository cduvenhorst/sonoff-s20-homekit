#ifndef __DEBUG_H__
#define __DEBUG_H__

#include <stdlib.h>
#include <stdio.h>

#ifdef SONOFF_DEBUG

#define DEBUG(message, ...) printf("["DEVICE_NAME" "DEVICE_MODEL"] %s: " message "\n", __func__, ##__VA_ARGS__)

#else

#define DEBUG(message, ...)

#endif

#endif				// __DEBUG_H__
