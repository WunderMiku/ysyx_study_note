#include "timer.h"
#include "npc.h"
#include <cassert>

uint64_t boot_time = 0;
uint64_t us = 0;
struct timespec now;
static uint64_t getTimeInternal() {
  clock_gettime(CLOCK_MONOTONIC_COARSE, &now);
  uint64_t us = now.tv_sec * 1000000 + now.tv_nsec / 1000;
  return us;
}
uint64_t getTime() {
  if (boot_time == 0) boot_time = getTimeInternal();
  uint64_t now = getTimeInternal();
  return now - boot_time;
}

tm getRealtime() {
	timespec ts;
	tm timeinfo;
	
	clock_gettime(CLOCK_REALTIME, &ts);
	localtime_r(&ts.tv_sec, &timeinfo);
	return timeinfo;
}

int realTimerRead(int raddr) {
  tm time_info = getRealtime();
	for(int i = 2; i < 8; i++) {
		if(raddr == RTC_ADDR + 4 * i) {
			switch(i) {
				case Sec:
					return time_info.tm_sec;	break;
				case Min:
					return time_info.tm_min;  break;
				case Hour:
					return time_info.tm_hour; break;
				case Day:
					return time_info.tm_mday; break;
				case Month:
					return time_info.tm_mon;  break;
				case Year:
					return time_info.tm_year + 1900; break;
			}
			break;
		}
	}
	// should not reach here
	assert(0);
}

int rtcRead(int raddr) {
	if(raddr == RTC_ADDR) {
		us = getTime();
		// printf("RTC: %lu\n", us);
		return (uint32_t)us;
	} else if(raddr == RTC_ADDR + 4) {
		assert(us != 0);
		// printf("RTC: %lu\n", us);
		return (uint32_t)(us >> 32);
	} else {
		// should not reach here
		assert(0);
	}
}

