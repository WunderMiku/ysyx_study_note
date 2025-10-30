#include "npc.h"
#include "timer.h"
#include <assert.h>
#include <stdio.h>

int deviceRead(int raddr) {
	if(raddr == RTC_ADDR || raddr == RTC_ADDR + 4) {
		return rtcRead(raddr);
	}
	
	if(raddr >= RTC_ADDR + 8 && raddr <= RTC_ADDR + 28) {
		return realTimerRead(raddr);
	}
	// 不属于任何设备寄存器，即非法访存
	printf("invalid device read address: 0x%08x\n", raddr);
	assert(0); 
}