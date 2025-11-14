#include "Vysyx_25090244_top.h"
#include <bits/posix2_lim.h>
#include <cassert>
#include <cstdint>
// #include <nvboard.h>
#include "config.h"
#include "funget.h"
#include "ringbuf.h"
#include "sdb/sdb.h"
#include "verilated.h"
#include "verilated_fst_c.h"
#include "svdpi.h"
#include "npc.h"
#include "files.h"
#include "ram.h"
#include "reg.h"
#include "watchpoint.h"
#include "difftest.h"
#include "timer.h"
#include "myVerilator.h"
#include "disasm.h"
#include <random>

#define MAX_SHOW_INST 50
#define MAX_SIM_TIME 100

vluint64_t simTime = 0;
vluint32_t* M = nullptr;
int instNum = 0;
uint32_t fileSize;
bool BatchMode = false;
char NEMU_SO_PATH[128] = "/home/misuzu/ysyx-workbench/nemu/build/riscv32-nemu-interpreter-so";

static void singleCycle(Vysyx_25090244_top*);
static void reset(Vysyx_25090244_top*);
static void execOnce();
static int checkEbreak();
static bool checkInstVaild();
static inline void clearScreen();
static void setInstLog(uint32_t n);
static void sim_init(int argc, char** argv);
void update_cpuState();
int get_random(int n);

std::unique_ptr<VerilatedContext> contextp{new VerilatedContext};
std::unique_ptr<Vysyx_25090244_top> dut{new Vysyx_25090244_top{contextp.get(), "TOP"}};

VerilatedFstC* tfp = new VerilatedFstC;
NpcState npcState;
CpuState cpu;

int main(int argc, char** argv) {
	fileSize = loadFile(argc, argv);

	// batch mode check
	BatchMode = (argc > 3 && (strcmp(argv[3], "--batch") == 0));
	
	verilatorInit(tfp, contextp.get(), dut.get(), argc, argv);
	reset(dut.get());
	
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

static void singleCycle(Vysyx_25090244_top* dut) {
  dut->clk = 1; dut->eval();
  dut->clk = 0; dut->eval();
	contextp->timeInc(1);
	tfp->dump(contextp->time());
}

static void reset(Vysyx_25090244_top *dut) {
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
      int n = get_random(7);
			// printf("n: " COLOR_MIKU "%d\n" COLOR_NONE, n);
			printf("Get ebreak: " COLOR_GREEN "HIT GOOD TRAP" COLOR_NONE\
				 ", at pc: 0x%08x\nTotal inst(s): " COLOR_MIKU "%d" COLOR_NONE "%s" "\nCiallo~(∠•ω＜)⌒☆\n" COLOR_NONE, dut->out_pc, instNum, COLOR_SELECT(n));
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

static bool checkInstVaild() {
	if(!(dut->inst_valid_flag)) {
		npcState.state = NPC_ABORT;
		printf(COLOR_RED "Invalid instruction:\t%s\n" COLOR_NONE, npcState.instLog);
		return false;
	}
	return true;
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
	if(checkEbreak()) return; // 检测 ebreak 指令
	if(!checkInstVaild()) return; // 检测 非法/未实现 指令

	// 执行指令
	singleCycle(dut.get());
	
	return;
}

void cpuExec(uint32_t n) {
	// printf("Running... for %d times\n", n);
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
		update_cpuState();
		uint32_t before_pc = dut->out_pc;
		execOnce();

#ifdef Difftest_enable
		difftest_step(dut->out_pc, before_pc);
#endif

#ifdef Ftrace_enable
		funget_detect(before_pc, dut->out_pc, npcState.inst); 
#endif
		// 执行后异常处理
		if(npcState.state == NPC_ABORT) ringbuf_print(&inst_ringbuf);
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

// 返回0～n之间的一个整数
int get_random(int n) {
	static std::random_device rd;
	static std::mt19937 gen(rd());
	static std::uniform_int_distribution<> dis(0, n); 

	return dis(gen);
}

static void sim_init(int argc, char** argv) {
	clearScreen();
	init_regex();
	init_disasm();
	update_cpuState();

#ifdef Difftest_enable
	init_difftest(NEMU_SO_PATH, fileSize, 1234);
#endif

#ifdef Ftrace_enable
	init_funget();
	get_function(argv[argc - 1]); // 最后传入ELF
#endif

#ifdef Watchpoint_enable
	init_wp_pool();
#endif
}

void update_cpuState() {
	cpu.pc = dut->out_pc;
	for(int i = 0; i < RISCV_GPR_NUM; i++) {
		cpu.gpr[i] = gpr(i);
	}
	for(int i = 0; i < CSR_COUNT; i++) {
		cpu.csr[i] = csr(i);
	}
}