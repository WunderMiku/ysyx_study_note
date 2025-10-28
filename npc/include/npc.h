#ifndef NPC_H__
#define NPC_H__

#include <stdint.h>
#include <time.h>

#define SERIAL_PORT  0x10000000
#define RTC_ADDR     0x10000060


uint64_t get_time();
static uint64_t get_time_internal();
tm get_realtime();

int device_read(int raddr);

#endif // NPC_H__