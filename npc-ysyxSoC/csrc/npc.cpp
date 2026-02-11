#include "VysyxSoCFull.h"
#include "verilated.h"
#include "verilated_fst_c.h"
#include "svdpi.h"
#include "npc.h"
#include "timer.h"
#include "reg.h"
#include "ram.h"
#include "watchpoint.h"
#include "sdb.h"
#include "disasm.h"
#include "ringbuf.h"

#include <cstdint>
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

void singleCycle(VysyxSoCFull* dut) {
  dut->clock = 1; dut->eval();
  dut->clock = 0; dut->eval();
	contextp->timeInc(1);
	tfp->dump(contextp->time());
}

void reset(VysyxSoCFull *dut, int n) {
	dut->reset = 1; dut->clock = 0; dut->eval();
	while(n--) {singleCycle(dut);}
	dut->reset = 0;	dut->eval();
	getTime();
}

// 检测是否结束
int checkEbreak() {
	svBit flag;
	dut->ebreak_get(&flag); 
	if(flag) {
		uint32_t A0 = dut->out_reg[10];
		if(!A0) {
      int n = get_random(7);
			// printf("n: " COLOR_MIKU "%d\n" COLOR_NONE, n);
			printf("Get ebreak: " COLOR_GREEN "HIT GOOD TRAP" COLOR_NONE\
				 ", at pc: 0x%08x\nTotal inst(s): " COLOR_MIKU "%d" COLOR_NONE " | Total cycle(s): " COLOR_MIKU "%d" COLOR_NONE "\nCPI: " COLOR_MIKU "%d" COLOR_NONE  "%s" "\nCiallo~(∠•ω＜)⌒☆\n" COLOR_NONE, dut->out_pc, instNum, cycNum, cycNum/instNum, COLOR_SELECT(n));
			npcState.state = NPC_END;
		} else {
			printf("Get ebreak: " COLOR_RED "HIT BAD TRAP  | A0 = 0x%08x \n > :(" COLOR_NONE\
				 ", at pc: 0x%08x\nTotal inst(s): " COLOR_MIKU "%d" COLOR_NONE "\n" COLOR_NONE, A0, dut->out_pc, instNum);
			npcState.state = NPC_ABORT;
		}
		return 1;
	} else {
		return 0;
	}
}

bool checkInstVaild() {
	if(!dut->inst_valid_flag) {
		npcState.state = NPC_ABORT;
		printf(COLOR_RED "Invalid instruction:\t%s\n" COLOR_NONE, npcState.instLog);
		return false;
	}
	return true;
}
void execOnce() {
		cycNum++; // 周期数加一
	if(dut->inst_done) { // 一条指令结束后
		instNum++;
		if(checkEbreak()) return; // 检测 ebreak 指令
		if(!checkInstVaild()) return; // 检测 非法/未实现 指令
	}

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
		update_cpuState();
		uint32_t before_pc = dut->out_pc;
		execOnce();

#ifdef Difftest_enable
		if(dut->inst_done){  // 每条指令结束后
			difftest_step(dut->out_pc, before_pc);
		}
		
#endif
		// 执行后异常处理
		if(npcState.state == NPC_ABORT) ringbuf_print(&inst_ringbuf);
		if(npcState.state != NPC_RUNNING) break;

		if(dut->inst_done){  // 每条指令结束后
#ifdef Ftrace_enable
			funget_detect(before_pc, dut->out_pc, npcState.inst); 
#endif

#ifdef Watchpoint_enable
			check_all_using_wp(); // 基于上一次指令执行结果进行检查
#endif
			setInstLog(n, before_pc); // 写入指令日志
		}
	}
	return;
}
static inline void clearScreen() {
	printf("\033[2J\033[H");
	return;
}


void setInstLog(uint32_t n, uint32_t before_pc) {
	npcState.inst = fetchInst(before_pc);
		char *p = npcState.instLog;
		int usedLen = sprintf(p, COLOR_MIKU "0x%08x"  COLOR_NONE ": |" COLOR_DYELLOW " %08x " COLOR_NONE "|  ", before_pc, npcState.inst);
		p += usedLen;
		disassemble(p, npcState.instLog + sizeof(npcState.instLog) - p, before_pc, (uint8_t*)&npcState.inst, 4);

		if(n < MAX_SHOW_INST) { printf("%s\n", npcState.instLog);}
		ringbuf_put(&inst_ringbuf, npcState.instLog);
}
void sim_init(int argc, char** argv) {
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
