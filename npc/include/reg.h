#ifndef __REG_H__
#define __REG_H__ 

#include <cassert>
#include "npc.h"
static inline int check_reg_idx(int idx) {
	assert(idx >= 0 && idx < RISCV_GPR_NUM);
  return idx;
}
extern const char *regs[];

#define gpr(idx) (dut->out_reg[check_reg_idx(idx)])

void isa_reg_display();
uint32_t isa_reg_str2val(const char *s, bool *success);

// mcycle test
static inline void read_mcycle(void);

#endif