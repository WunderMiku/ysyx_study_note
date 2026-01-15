#ifndef __REG_H__
#define __REG_H__ 

#include <cassert>
#include "config.h"
#include "npc.h"
static inline int check_reg_idx(int idx) {
	assert(idx >= 0 && idx < RISCV_GPR_NUM);
  return idx;
}

static inline int check_csr_idx(int idx) {
	assert(idx >= 0 && idx < CSR_COUNT);
  return idx;
}
extern const char *regs[];
extern const char *csrs[];

#define gpr(idx) (dut->out_reg[check_reg_idx(idx)])
#define csr(idx) (dut->out_csr[check_csr_idx(idx)])

void isa_reg_display();
uint32_t isa_reg_str2val(const char *s, bool *success);

// mcycle test
static inline void read_mcycle(void);

#endif