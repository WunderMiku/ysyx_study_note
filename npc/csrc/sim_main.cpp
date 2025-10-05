#include "Vtop.h"
#include <bits/posix2_lim.h>
#include <nvboard.h>
#include "verilated.h"
#include "verilated_fst_c.h"
#include "verilatedos.h"
#include "svdpi.h"


#define MAX_SIM_TIME 50

void nvboard_bind_all_pins(TOP_NAME* top);
// extern svBit ebreak_get();

vluint64_t sim_time = 0;
vluint32_t M[1024] = {0x01400513, 0x010000e7, 0x00c000e7, 0x00100073, 0x00a50513, 0x00008067};

static void single_cycle(Vtop*);
static void reset(Vtop*);
int main(int argc, char** argv) {
	if (false && argc && argv) {} // Prevent unused variable warnings
	Verilated::mkdir("logs"); // Create a directory for logs
	const std::unique_ptr<VerilatedContext> contextp{new VerilatedContext};
	const std::unique_ptr<Vtop> dut{new Vtop{contextp.get(), "TOP"}};
	contextp->debug(0);
	contextp->traceEverOn(true); // Enable tracing
	contextp->commandArgs(argc, argv);

	VerilatedFstC* tfp = new VerilatedFstC;
	dut->trace(tfp, 5);  // 设置跟踪深度
	tfp->open("logs/sim.fst");  // 打开并创建波形文件

	// 得到DPI函数导入需要的scope
	const svScope scope = svGetScopeFromName("TOP.top.uEXU");  
	assert(scope);
	svSetScope(scope);
	// nvboard_bind_all_pins(dut.get());
  // nvboard_init();
	reset(dut.get());
	int i = 0;
	contextp->timeInc(1);
	tfp->dump(contextp->time());
	// nvboard_update();
	while(i < 30) {
		// nvboard_update();
		single_cycle(dut.get());
		contextp->timeInc(1);
		tfp->dump(contextp->time());
		i++;

		// 检测是否结束
		svBit flag;
		dut->ebreak_get(&flag); 
		if(flag) {
			// nvboard_quit();
			printf("Simulation Ended by ebreak\n");
			break;
		}
	}

	dut->final();
	tfp->close();
  // nvboard_quit();
	

	return 0;
}

static void single_cycle(Vtop* dut) {
  dut->clk = 0; dut->eval();
	dut->inst = M[(dut->pc_out) >> 2]; dut->eval();
	printf("pc: %x, inst: %x\n", dut->pc_out, dut->inst);
  dut->clk = 1; dut->eval();
	// printf("a0: %x08\n", dut->a0);
}

static void reset(Vtop *dut) {
	dut->rst = 1;
	single_cycle(dut);
	dut->rst = 0;
}