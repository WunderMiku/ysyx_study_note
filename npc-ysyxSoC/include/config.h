#ifndef __CONFIG_H__
#define __CONFIG_H__

#define MEM_BASE 0x20000000
#define MEM_SIZE 0x1000

#define MROM_BASE 0x20000000

#define FLASH_BASE 0x30000000
#define FLASH_SIZE 0x10000000

// device
#define SERIAL_PORT  0x10000000
#define RTC_ADDR     0x0200BFF8

#define RISCV_GPR_NUM 32
#define CSR_COUNT 4

#define MAX_SHOW_INST 2000

// #define Ftrace_enable
#define Watchpoint_enable
// #define Difftest_enable
// #define Mtrace_enable
// #define InstLog_enable

#define FLASH_ROM

#endif
