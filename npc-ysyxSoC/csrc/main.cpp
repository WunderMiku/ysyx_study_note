#include "VysyxSoCFull.h"
#include <bits/posix2_lim.h>
#include <cassert>
#include <cstdint>
#include "config.h"
#include "verilated.h"
#include "verilated_fst_c.h"
#include "npc.h"
#include "files.h"
#include "sdb.h"
#include "verilatedos.h"


#define MAX_SIM_TIME 100

vluint64_t simTime = 0;
vluint32_t* M = nullptr;
vluint32_t* Flash = nullptr;

int instNum = 0;
uint32_t fileSize;
bool BatchMode = false;
char NEMU_SO_PATH[128] = "/home/misuzu/ysyx-workbench/nemu/build/riscv32-nemu-interpreter-so";

std::unique_ptr<VerilatedContext> contextp{new VerilatedContext};
std::unique_ptr<VysyxSoCFull> dut{new VysyxSoCFull{contextp.get(), "TOP"}};

VerilatedFstC* tfp = new VerilatedFstC;

NpcState npcState;
CpuState cpu;

int main(int argc, char** argv) {
	setbuf(stdout, NULL); // 取消缓冲区
	fileSize = loadFile(argc, argv);
	// batch mode check
	BatchMode = (argc > 3 && (strcmp(argv[3], "--batch") == 0));
	
	verilatorInit(tfp, contextp.get(), dut.get(), argc, argv);
	reset(dut.get(), 15);
	
	contextp->timeInc(1);
	tfp->dump(contextp->time());
	npcState.state = NPC_RUNNING;
	sim_init(argc, argv);

	if(BatchMode) { 
		printf("[BATMODE] Running...\n");
		cpuExec(-1);
	} else {
		while(1){
			sdbMainLoop();
			if(npcState.state != NPC_RUNNING) break;
		}
	}

	dut->final();
	tfp->close();

	if(npcState.state == NPC_END || npcState.state == NPC_QUIT) {
		return 0;
	} else {
		return 1;
	}
}