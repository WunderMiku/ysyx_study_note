#ifndef __NPC_H__
#define __NPC_H__

#include "Vtop.h"
#include "verilatedos.h"
#include <stdint.h>

#define COLOR_RED    "\033[1;31m"
#define COLOR_GREEN  "\033[1;32m"
#define COLOR_YELLOW "\033[1;33m"
#define COLOR_BLUE   "\033[44m"
#define COLOR_BLACK   "\33[1;30m"
#define COLOR_MAGENTA "\33[1;35m"
#define COLOR_CYAN    "\33[1;36m"
#define COLOR_WHITE   "\33[1;37m"
#define COLOR_NONE   "\033[0m"

#define COLOR_BG_BLACK   "\33[1;40m"
#define COLOR_BG_RED     "\33[1;41m"
#define COLOR_BG_GREEN   "\33[1;42m"
#define COLOR_BG_YELLOW  "\33[1;43m"
#define COLOR_BG_BLUE    "\33[1;44m"
#define COLOR_BG_MAGENTA "\33[1;45m"
#define COLOR_BG_CYAN    "\33[1;46m"
#define COLOR_BG_WHITE   "\33[1;47m"
#define COLOR_BG_LYELLOW "\033[103m"
#define COLOR_BG_LGREEN  "\033[102m"

#define COLOR_MIKU "\033[38;2;39;197;187m"
#define COLOR_DYELLOW "\033[38;2;255;199;6m"

#define BOLD_TEXT "\033[1m"
#define ITALIC_TEXT "\033[3m"

#define MEM_BASE 0x80000000
#define MEM_SIZE 0x8000000

#define SERIAL_PORT  0x10000000
#define RTC_ADDR     0x10000060

typedef enum {
	NPC_RUNNING = 0,
	NPC_END,
	NPC_QUIT,
	NPC_ABORT
} NpcStateType;

typedef struct {
  NpcStateType state;
	uint32_t inst;
	char instLog[256];
} NpcState;

extern NpcState npcState;
extern std::unique_ptr<Vtop> dut;

extern vluint32_t* M;
void cpuExec(uint32_t n);

#endif // NPC_H__