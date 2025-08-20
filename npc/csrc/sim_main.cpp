#include "Vtop.h"
#include "verilated.h"

#define MAX_SIM_TIME 50

vluint64_t sim_time = 0;

VerilatedContext* contextp = new VerilatedContext;

Vtop* top = new Vtop{contextp};
int get_truetable(int a, int b) {
	top->a = a;
	top->b = b;
	top->eval();
	return top->f;
}
int main(int argc, char** argv) {
	contextp->commandArgs(argc, argv);
	
	
	for (int a = 0; a < 2; a++) {
        for (int b = 0; b < 2; b++) {
			int result = get_truetable(a, b);
			printf("a: %d, b: %d, f: %d\n", a, b, result);
		}
	}
	
	delete top;
	delete contextp;
	return 0;
}
