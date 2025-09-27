#include "Vtop.h"
#include <nvboard.h>
#include "verilated.h"
#include "verilated_fst_c.h"

#define MAX_SIM_TIME 50

void nvboard_bind_all_pins(TOP_NAME* top);

vluint64_t sim_time = 0;

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

		nvboard_bind_all_pins(dut.get());
  	nvboard_init();

	while(1) {
		nvboard_update();
		single_cycle(dut.get());
	}
	
	tfp->close();
  nvboard_quit();
	

	return 0;
}

static void single_cycle(Vtop* dut) {
  dut->clk = 0; dut->eval();
  dut->clk = 1; dut->eval();
}

static void reset(Vtop *dut) {
	dut->rst = 1;
	single_cycle(dut);
	dut->rst = 0;
}