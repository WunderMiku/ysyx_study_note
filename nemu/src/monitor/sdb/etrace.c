

#include "utils.h"
#include <stdint.h>
#include <stdio.h>
void etrace(uint32_t before_pc, uint32_t pc, uint32_t inst) {
	if(inst == 0x73) {
		printf(ANSI_FG_MAGENTA"[ecall]"ANSI_NONE" pc: "ANSI_FG_CYAN"0x%08x"ANSI_NONE" -> "ANSI_FG_YELLOW"0x%08x\n"ANSI_NONE, before_pc, pc);
	} 

	else if(inst == 0x30200073) {
		printf(ANSI_FG_BLUE"[mret]"ANSI_NONE" pc: "ANSI_FG_YELLOW"0x%08x"ANSI_NONE" <- "ANSI_FG_CYAN"0x%08x"ANSI_NONE"\n", pc, before_pc);
	}
}
