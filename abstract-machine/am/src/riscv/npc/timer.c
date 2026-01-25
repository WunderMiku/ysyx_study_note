#include "riscv/riscv.h"
#include "riscv/npc/include/npc.h"
#include <am.h>
#include <stdio.h>

void __am_timer_init() {
}

void __am_timer_uptime(AM_TIMER_UPTIME_T *uptime) {
  uint32_t low, high1, high2;
  // 64bits 原子读取方法
  do {
    high1 = inl(RTC_ADDR + 4);
    low = inl(RTC_ADDR);
    high2 = inl(RTC_ADDR + 4);
  } while (high1 != high2);

  uint64_t us = ((uint64_t)high1 << 32) | low;
  uptime->us = us;
}

void __am_timer_rtc(AM_TIMER_RTC_T *rtc) {
  // rtc->second = inl(RTC_ADDR + 4 * 2);
  // rtc->minute = inl(RTC_ADDR + 4 * 3);
  // rtc->hour   = inl(RTC_ADDR + 4 * 4);
  // rtc->day    = inl(RTC_ADDR + 4 * 5);
  // rtc->month  = inl(RTC_ADDR + 4 * 6);
  // rtc->year   = inl(RTC_ADDR + 4 * 7);
  rtc->second = 0;
  rtc->minute = 0;
  rtc->hour   = 0;
  rtc->day    = 1;
  rtc->month  = 1;
  rtc->year   = 1970;
}
