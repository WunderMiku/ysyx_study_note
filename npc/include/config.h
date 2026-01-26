#ifndef __CONFIG_H__
#define __CONFIG_H__

#define MEM_BASE 0x80000000
#define MEM_SIZE 0x8000000

#define SERIAL_PORT  0x10000000
#define RTC_ADDR     0x0200BFF8

#define RISCV_GPR_NUM 32
#define CSR_COUNT 4

// #define Ftrace_enable
#define Watchpoint_enable
// #define Difftest_enable
// #define Mtrace_enable
#define BatchMode_enable

#endif