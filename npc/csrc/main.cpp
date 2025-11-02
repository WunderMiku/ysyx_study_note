#include "Vtop.h"
#include <bits/posix2_lim.h>
#include <cassert>
#include <cstdint>
#include <nvboard.h>
#include "funget.h"
#include "ringbuf.h"
#include "sdb/sdb.h"
#include "verilated.h"
#include "verilated_fst_c.h"
#include "svdpi.h"
#include "npc.h"
#include "files.h"
#include "ram.h"
#include "watchpoint.h"
#include "timer.h"
#include "myVerilator.h"
#include "disasm.h"

#define MAX_SHOW_INST 50
#define MAX_SIM_TIME 100

vluint64_t simTime = 0;
vluint32_t* M = nullptr;
int instNum = 0;

static void singleCycle(Vtop*);
static void reset(Vtop*);
static void execOnce();
static int checkEbreak();
static inline void clearScreen();
static void setInstLog(uint32_t n);

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
	clearScreen();
	init_regex();
	init_disasm();

#ifdef Ftrace_enable
	init_funget();
	get_function(argv[argc - 1]); // 最后传入ELF
#endif

#ifdef Watchpoint_enable
	init_wp_pool();
#endif

#ifdef BATMODE // 批处理模式
	cpuExec(-1);
#else
	while(1){
		sdbMainLoop();
		if(npcState.state != NPC_RUNNING) break;
	}

	if(npcState.state == NPC_ABORT) ringbuf_print(&inst_ringbuf);
#endif

	dut->final();
	tfp->close();

	return 0;
}

static void singleCycle(Vtop* dut) {
  dut->clk = 1; dut->eval();
  dut->clk = 0; dut->eval();
	contextp->timeInc(1);
	tfp->dump(contextp->time());
}

static void reset(Vtop *dut) {
	dut->rst = 1; dut->clk = 0; dut->eval();
	singleCycle(dut);
	dut->rst = 0;	dut->eval();
	getTime();
}

// 检测是否结束
static int checkEbreak() {
	svBit flag;
	dut->ebreak_get(&flag); 
	if(flag) {
		if(!dut->A0) {
			printf("Get ebreak: " COLOR_GREEN "HIT GOOD TRAP" COLOR_NONE\
				 ", at pc: 0x%08x\nTotal inst(s): " COLOR_MIKU "%d" COLOR_NONE "\nCiallo~(∠•ω＜)⌒☆\n", dut->out_pc, instNum);
			npcState.state = NPC_END;
		} else {
			printf("Get ebreak: " COLOR_RED "HIT BAD TRAP" COLOR_NONE\
				 ", at pc: 0x%08x\nTotal inst(s): " COLOR_MIKU "%d" COLOR_NONE "\n" COLOR_NONE, dut->out_pc, instNum);
			npcState.state = NPC_ABORT;
		}
		return 1;
	} else {
		return 0;
	}
}
static void execOnce() {
	// Ram 读写端口数据处理
	if(dut->ramRe) { // ram读
		dut->ramReadData = pmem_read(dut->ramReadAddr, 4);
	}

	if(dut->ramWe) { // ram写
		pmem_write(dut->ramWriteAddr, dut->ramWriteData, dut->ramWriteMask);
	}

	instNum++;
	if(checkEbreak()) return;
	// ================ 控制信号更新完成，步进以执行该周期指令 ===============
	// 电路步进
	singleCycle(dut.get());
	
	return;
}

void cpuExec(uint32_t n) {
	if(npcState.state == NPC_STOP) {npcState.state = NPC_RUNNING;} // 恢复执行
	if(npcState.state != NPC_RUNNING) {
		printf("NPC is not running, press 'q' to quit\n");
		return;
	}
	for(int i = 0; i < n; i++) {
#ifdef Watchpoint_enable
		check_all_using_wp(); // 基于上一次指令执行结果进行检查
#endif
		setInstLog(n);
		uint32_t before_pc = dut->out_pc;
		execOnce();

#ifdef Ftrace_enable
		funget_detect(before_pc, dut->out_pc, npcState.inst); 
#endif

		if(npcState.state != NPC_RUNNING) break;
	}
	return;
}
static inline void clearScreen() {
	printf("\033[2J\033[H");
	return;
}

static void setInstLog(uint32_t n) {
	npcState.inst = fetchInst(dut->out_pc);
		char *p = npcState.instLog;
		int usedLen = sprintf(p, COLOR_MIKU "0x%08x"  COLOR_NONE ": |" COLOR_DYELLOW " %08x " COLOR_NONE "|  ", dut->out_pc, npcState.inst);
		p += usedLen;
		disassemble(p, npcState.instLog + sizeof(npcState.instLog) - p, dut->out_pc, (uint8_t*)&npcState.inst, 4);

		if(n < MAX_SHOW_INST) { printf("%s\n", npcState.instLog);}
		ringbuf_put(&inst_ringbuf, npcState.instLog);
}