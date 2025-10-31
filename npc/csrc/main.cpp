#include "Vtop.h"
#include <bits/posix2_lim.h>
#include <cassert>
#include <nvboard.h>
#include "sdb/sdb.h"
#include "verilated.h"
#include "verilated_fst_c.h"
#include "svdpi.h"
#include "npc.h"
#include "files.h"
#include "ram.h"
#include "timer.h"
#include "myVerilator.h"

#define MAX_SIM_TIME 100

vluint64_t simTime = 0;
vluint32_t* M = nullptr;
int instNum = 0;

static void singleCycle(Vtop*);
static void reset(Vtop*);
static void execOnce();
static int checkEbreak();

std::unique_ptr<VerilatedContext> contextp{new VerilatedContext};
std::unique_ptr<Vtop> dut{new Vtop{contextp.get(), "TOP"}};

VerilatedFstC* tfp = new VerilatedFstC;

NpcState npcState;
int main(int argc, char** argv) {
	loadFile(argc, argv);
	
	verilatorInit(tfp, contextp.get(), dut.get(), argc, argv);
	reset(dut.get());
	
	contextp->timeInc(1);
	tfp->dump(contextp->time());
	npcState.state = NPC_RUNNING;
	while(1){
		// if(execOnce()) break;
		sdbMainLoop();
		if(npcState.state != NPC_RUNNING) break;
	}

	dut->final();
	tfp->close();

	return 0;
}

static void singleCycle(Vtop* dut) {
  dut->clk = 0; dut->eval();
  dut->clk = 1; dut->eval();

	contextp->timeInc(1);
	tfp->dump(contextp->time());
}

static void reset(Vtop *dut) {
	dut->rst = 1;
	singleCycle(dut);
	dut->rst = 0;
	dut->eval();
	getTime();
}

// 检测是否结束
static int checkEbreak() {
	svBit flag;
	dut->ebreak_get(&flag); 
	if(flag) {
		if(!dut->A0) {
			printf("Get ebreak: " COLOR_GREEN "HIT GOOD TRAP" COLOR_NONE ", Total inst(s): %d\n Ciallo~(∠•ω＜)⌒☆\n", instNum);
		} else {
			printf("Get ebreak: " COLOR_RED "HIT BAD TRAP" COLOR_NONE ", Total inst(s): %d\n" COLOR_NONE, instNum);
		}
		return 1;
	} else {
		return 0;
	}
}
static void execOnce() {
	// Ram 读写端口数据处理
	// printf("pc :0x%08x\n", dut->out_pc);
	if(dut->ramRe) { // ram读
		dut->ramReadData = pmem_read(dut->ramReadAddr, 4);
	}

	if(dut->ramWe) { // ram写
		pmem_write(dut->ramWriteAddr, dut->ramWriteData, dut->ramWriteMask);
	}

	instNum++;
	if(checkEbreak()) {
		npcState.state = NPC_END;
		return;
	}
	// 电路步进
	singleCycle(dut.get());
	
	return;
}

void cpuExec(uint32_t n) {
	if(npcState.state != NPC_RUNNING) {
		printf("NPC is not running, press 'q' to quit\n");
		return;
	}
	for(int i = 0; i < n; i++) {
		execOnce();
		if(npcState.state != NPC_RUNNING) break;
	}
	return;
}