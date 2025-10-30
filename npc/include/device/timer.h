#ifndef __TIMER_H__
#define __TIMER_H__ 

#include <stdint.h>
#include <time.h>

typedef enum {
	Sec = 2,
	Min,
	Hour,
	Day,
	Month,
	Year
} TimeInfo;

uint64_t getTime();
static uint64_t getTimeInternal();
tm getRealtime();

int realTimerRead(int raddr);
int rtcRead(int raddr);

#endif