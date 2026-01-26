#include "VysyxSoCFull.h"
#include "verilated.h"
#include "verilated_fst_c.h"
#include "svdpi.h"
#include "npc.h"

#include <random>

void verilatorInit(VerilatedFstC* tfp, VerilatedContext* contextp, VysyxSoCFull* dut, int argc, char** argv) {
	Verilated::mkdir("logs"); // Create a directory for logs
	contextp->debug(0);
	contextp->traceEverOn(true); // Enable tracing
	contextp->commandArgs(argc, argv);

	dut->trace(tfp, 5);  // 设置跟踪深度
	tfp->open("logs/sim.fst");  // 打开并创建波形文件

	// 得到DPI函数导入需要的scope
	const svScope scope = svGetScopeFromName("TOP.ysyxSoCFull.asic.cpu.cpu.uEXU");  
	assert(scope);
	svSetScope(scope);
}

// 返回0～n之间的一个整数
int get_random(int n) {
	static std::random_device rd;
	static std::mt19937 gen(rd());
	static std::uniform_int_distribution<> dis(0, n); 

	return dis(gen);
}
int checkEbreak() {
	svBit flag;
	dut->ebreak_get(&flag); 
	if(flag) {
		int n = get_random(7);
		printf("Get ebreak: " COLOR_GREEN "HIT GOOD TRAP" COLOR_NONE "%s" "\nCiallo~(∠•ω＜)⌒☆\n" COLOR_NONE, COLOR_SELECT(n));
		return 1;
	} else {
		return 0;
	}
}

void singleCycle(VysyxSoCFull* dut) {
  dut->clock = 1; dut->eval();
  dut->clock = 0; dut->eval();
	contextp->timeInc(1);
	tfp->dump(contextp->time());
}

void reset(VysyxSoCFull *dut) {
	dut->reset = 1; dut->clock = 0; dut->eval();
	singleCycle(dut);
	singleCycle(dut);
	dut->reset = 0;	dut->eval();
}

void execOnce() {
	singleCycle(dut.get());
	return;
}

void cpuExec(uint32_t n) {
	for(int i = 0; i < n; i++) {
		execOnce();
		if(checkEbreak()) {break;}
	}
	return;
}
static inline void clearScreen() {
	printf("\033[2J\033[H");
	return;
}
void sim_init(int argc, char** argv) {
	clearScreen();
}


