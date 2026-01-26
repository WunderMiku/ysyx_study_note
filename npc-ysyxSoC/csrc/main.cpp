#include "VysyxSoCFull.h"
#include <bits/posix2_lim.h>
#include <cassert>
#include <cstdint>
#include "verilated.h"
#include "verilated_fst_c.h"
#include "svdpi.h"
#include "npc.h"
#include "files.h"

#include <random>

#define MAX_SHOW_INST 50
#define MAX_SIM_TIME 100

#define MROM_BASE 0x20000000

vluint64_t simTime = 0;
vluint32_t* M = nullptr;

std::unique_ptr<VerilatedContext> contextp{new VerilatedContext};
std::unique_ptr<VysyxSoCFull> dut{new VysyxSoCFull{contextp.get(), "TOP"}};

VerilatedFstC* tfp = new VerilatedFstC;

extern "C" void flash_read(int32_t addr, int32_t *data) { assert(0); }
extern "C" void mrom_read(int32_t addr, int32_t *data) { 
	int32_t M_addr = (addr - MROM_BASE) >> 2;
	*data = M[M_addr];
	// printf("mrom_read: 0x%08x\n", *data);
}
int main(int argc, char** argv) {
	setbuf(stdout, NULL);
	verilatorInit(tfp, contextp.get(), dut.get(), argc, argv);
	reset(dut.get());
	
	contextp->timeInc(1);
	tfp->dump(contextp->time());
	sim_init(argc, argv);
	loadFile(argc, argv);
	cpuExec(-1);

	dut->final();
	tfp->close();

	return 0;
}


