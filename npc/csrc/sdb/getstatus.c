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

	printf("TOP Module State: ");
	switch(dut->out_top_state) {
		case 0: printf(COLOR_GREEN "IDLE\n" COLOR_NONE); break;
		case 1: printf(COLOR_YELLOW "MEM_WAIT\n" COLOR_NONE); break;

		default: printf(COLOR_RED "UNKNOWN\n" COLOR_NONE); break;
	}

	printf("\n");
	
	printf(COLOR_DYELLOW "===== Submodules Status =====\n" COLOR_NONE);

	printf("IFU State: ");
	switch(dut->out_ifu_state) {
		case 0: printf(COLOR_GREEN "IDLE\n" COLOR_NONE); break;
		case 1: printf(COLOR_YELLOW "WAIT\n" COLOR_NONE); break;

		default: printf(COLOR_RED "UNKNOWN\n" COLOR_NONE); break;
	}

	printf("LSU State: ");
	switch(dut->out_lsu_state) {
		case 0: printf(COLOR_GREEN "IDLE\n" COLOR_NONE); break;
		case 1: printf(COLOR_YELLOW "WAIT\n" COLOR_NONE); break;

		default: printf(COLOR_RED "UNKNOWN\n" COLOR_NONE); break;
	}
}