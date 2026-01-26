#include "npc.h"

void print_all_status() {
	// printf(COLOR_DYELLOW "===== NPC Status =====\n" COLOR_NONE);
	// printf("NPC State: ");
	// switch(npcState.state) {
	// 	case NPC_RUNNING: printf(COLOR_GREEN "RUNNING\n" COLOR_NONE); break;
	// 	case NPC_STOP:    printf(COLOR_YELLOW "STOP\n" COLOR_NONE); break;
	// 	case NPC_QUIT:    printf(COLOR_RED "QUIT\n" COLOR_NONE); break;
	// 	case NPC_ABORT:   printf(COLOR_RED "ABORT\n" COLOR_NONE); break;
	// 	case NPC_END:     printf(COLOR_MIKU "END\n" COLOR_NONE); break;
	// 	default:          printf(COLOR_RED "UNKNOWN\n" COLOR_NONE); break;
	// }
	// printf("PC: 0x%08x\n", dut->out_pc);

	// printf("TOP Module State: ");
	// switch(dut->out_top_state) {
	// 	case 0: printf(COLOR_GREEN "IFU_WAIT\n" COLOR_NONE); break;
	// 	case 1: printf(COLOR_YELLOW "EXU_WAIT\n" COLOR_NONE); break;
	// 	case 2: printf(COLOR_YELLOW "LSU_WAIT\n" COLOR_NONE); break;
	// 	case 3: printf(COLOR_MIKU "WBU_WAIT\n" COLOR_NONE); break;

	// 	default: printf(COLOR_RED "UNKNOWN\n" COLOR_NONE); break;
	// }

	// printf("\n");
	
	// printf(COLOR_DYELLOW "===== Submodules Status =====\n" COLOR_NONE);

	// printf("IFU State: ");
  // switch(dut->out_ifu_state) {
	// 	case 0: printf(COLOR_GREEN "IDLE\n" COLOR_NONE); break;
	// 	case 1: printf(COLOR_YELLOW "WAIT_RESP\n" COLOR_NONE); break;
	// 	case 2: printf(COLOR_MIKU "WAIT_R\n" COLOR_NONE); break;

	// 	default: printf(COLOR_RED "UNKNOWN\n" COLOR_NONE); break;
	// }

	// printf("LSU State: ");
	// printf("(" COLOR_MAGENTA "Write" COLOR_NONE ") ");
	// switch(dut->out_lsu_write_state) {
	// 	case 0: printf(COLOR_GREEN "IDLE" COLOR_NONE); break;
	// 	case 1: printf(COLOR_YELLOW "WAIT_AW" COLOR_NONE); break;
	// 	case 2: printf(COLOR_YELLOW "WAIT_W" COLOR_NONE); break;
	// 	case 3: printf(COLOR_MIKU "WAIT_B" COLOR_NONE); break;

	// 	default: printf(COLOR_RED "UNKNOWN\n" COLOR_NONE); break;
	// }
	// printf("  |  ");
	// printf("(" COLOR_MIKU "Read" COLOR_NONE ") ");
	// switch(dut->out_lsu_read_state) {
	// 	case 0: printf(COLOR_GREEN "IDLE\n" COLOR_NONE); break;
	// 	case 1: printf(COLOR_YELLOW "WAIT_RESP\n" COLOR_NONE); break;
	// 	case 2: printf(COLOR_MIKU "WAIT_R\n" COLOR_NONE); break;

	// 	default: printf(COLOR_RED "UNKNOWN\n" COLOR_NONE); break;
	// }
}