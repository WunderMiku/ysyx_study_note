#ifndef __MY_VERILATOR_H__
#define __MY_VERILATOR_H__

#include "Vysyx_25090244_top.h"
#include "verilated.h"
#include "verilated_fst_c.h"

void verilatorInit(VerilatedFstC* tfp, VerilatedContext* contextp, Vysyx_25090244_top* dut, int argc, char** argv);

#endif