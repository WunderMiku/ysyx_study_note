#include "riscv/riscv.h"
#include "riscv/npc/include/npc.h"
#include <am.h>

void __am_timer_init() {
}

void __am_timer_uptime(AM_TIMER_UPTIME_T *uptime) {
  uint32_t low = inl(RTC_ADDR);
  uint32_t high = inl(RTC_ADDR + 4);
  uint64_t us = ((uint64_t)high << 32) | low;
  uptime->us = us;
}

void __am_timer_rtc(AM_TIMER_RTC_T *rtc) {
  rtc->second = inl(RTC_ADDR + 4 * 2);
  rtc->minute = inl(RTC_ADDR + 4 * 3);
  rtc->hour   = inl(RTC_ADDR + 4 * 4);
  rtc->day    = inl(RTC_ADDR + 4 * 5);
  rtc->month  = inl(RTC_ADDR + 4 * 6);
  rtc->year   = inl(RTC_ADDR + 4 * 7);
}
