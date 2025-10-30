#ifndef __MY_VERILATOR_H__
#define __MY_VERILATOR_H__

#include "Vtop.h"
#include "verilated.h"
#include "verilated_fst_c.h"

void verilatorInit(VerilatedFstC* tfp, VerilatedContext* contextp, Vtop* dut, int argc, char** argv);

#endif