#include "Vtop.h"
#include "verilated.h"
#include "verilated_fst_c.h"

#define MAX_SIM_TIME 50

vluint64_t sim_time = 0;


int get_truetable(Vtop* top, int a, int b);
int main(int argc, char** argv) {
	const std::unique_ptr<VerilatedContext> contextp{new VerilatedContext};
	const std::unique_ptr<Vtop> top{new Vtop{contextp.get(), "TOP"}};

	if (false && argc && argv) {} // Prevent unused variable warnings
	Verilated::mkdir("logs"); // Create a directory for logs
	contextp->debug(0);
	contextp->traceEverOn(true); // Enable tracing
	contextp->commandArgs(argc, argv);

	VerilatedFstC* tfp = new VerilatedFstC;
	top->trace(tfp, 5);  // 设置跟踪深度
	tfp->open("logs/sim.fst");  // 打开并创建波形文件

	
	for (int a = 0; a < 2; a++) {
        for (int b = 0; b < 2; b++) {
			int result = get_truetable(top.get(), a, b);
			printf("a: %d, b: %d, f: %d\n", a, b, result);
			contextp->timeInc(1);
			tfp->dump(contextp->time());
		}
	}
	
	// 额外的时间步进以确保所有数据都被展示
	contextp->timeInc(1);
	tfp->dump(contextp->time());
	top->final();
	tfp->close();
	return 0;
}

int get_truetable(Vtop* top, int a, int b) {
	top->a = a;
	top->b = b;
	top->eval();
	return top->f;
}