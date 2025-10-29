#include "../include/npc.h"
#include <assert.h>
#include <stdio.h>

typedef enum {
	Sec = 2,
	Min,
	Hour,
	Day,
	Month,
	Year
} Time_info;

uint64_t us = 0;
int device_read(int raddr) {
	if(raddr == RTC_ADDR || raddr == RTC_ADDR + 4) {
		if(raddr == RTC_ADDR) {
			us = get_time();
			// printf("RTC: %lu\n", us);
			return (uint32_t)us;
		} else {
			assert(us != 0);
			// printf("RTC: %lu\n", us);
			return (uint32_t)(us >> 32);
		}
	}
	
	if(raddr >= RTC_ADDR + 8 && raddr <= RTC_ADDR + 28) {
		tm time_info = get_realtime();
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
	}
	// 不属于任何设备寄存器，即非法访存
	printf("invalid device read address: 0x%08x\n", raddr);
	assert(0); 
}