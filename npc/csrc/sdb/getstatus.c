#include "npc.h"

void print_all_status() {
	printf(COLOR_DYELLOW "===== NPC Status =====\n" COLOR_NONE);
	printf("NPC State: ");
	switch(npcState.state) {
		case NPC_RUNNING: printf(COLOR_GREEN "RUNNING\n" COLOR_NONE); break;
		case NPC_STOP:    printf(COLOR_YELLOW "STOP\n" COLOR_NONE); break;
		case NPC_QUIT:    printf(COLOR_RED "QUIT\n" COLOR_NONE); break;
		case NPC_ABORT:   printf(COLOR_RED "ABORT\n" COLOR_NONE); break;
		default:          printf(COLOR_RED "UNKNOWN\n" COLOR_NONE); break;
	}
	printf("PC: 0x%08x\n", dut->out_pc);

	printf(COLOR_DYELLOW "===== Module Status =====\n" COLOR_NONE);
	printf("IFU Valid: ");
	if(dut->IFU_valid_flag) {
		printf(COLOR_GREEN "True\n" COLOR_NONE);
	} else {
		printf(COLOR_RED "False\n" COLOR_NONE);
	}
}