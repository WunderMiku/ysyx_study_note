#include "myVerilator.h"
void verilatorInit(VerilatedFstC* tfp, VerilatedContext* contextp, Vtop* dut, int argc, char** argv) {
	Verilated::mkdir("logs"); // Create a directory for logs
	contextp->debug(0);
	contextp->traceEverOn(true); // Enable tracing
	contextp->commandArgs(argc, argv);

	dut->trace(tfp, 5);  // 设置跟踪深度
	tfp->open("logs/sim.fst");  // 打开并创建波形文件

	// 得到DPI函数导入需要的scope
	const svScope scope = svGetScopeFromName("TOP.top.uEXU");  
	assert(scope);
	svSetScope(scope);
}