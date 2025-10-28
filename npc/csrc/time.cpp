#include "../include/npc.h"

uint64_t boot_time = 0;
struct timespec now;
static uint64_t get_time_internal() {
  clock_gettime(CLOCK_MONOTONIC_COARSE, &now);
  uint64_t us = now.tv_sec * 1000000 + now.tv_nsec / 1000;
  return us;
}
uint64_t get_time() {
  if (boot_time == 0) boot_time = get_time_internal();
  uint64_t now = get_time_internal();
  return now - boot_time;
}

tm get_realtime() {
	timespec ts;
	tm timeinfo;  // 这就是 timeinfo 变量
	
	clock_gettime(CLOCK_REALTIME, &ts);
	localtime_r(&ts.tv_sec, &timeinfo);
	return timeinfo;
}


