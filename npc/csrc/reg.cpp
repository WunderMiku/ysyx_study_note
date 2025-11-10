#include "macro.h"
#include "npc.h"
#include "reg.h"
#include <assert.h>
// #include <stdio.h>
// #include <string.h>

const char *regs[] = {
  "$0", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
  "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
  "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
  "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"
};

const char *csrs[] = {
  "mepc", "mstatus", "mcause", "mtvec"
};
void isa_reg_display() {
  printf(COLOR_DYELLOW "\n===== Registers =====\n" COLOR_NONE);
  for (int i = 0; i < ARRLEN(regs); i++) {
    printf(COLOR_YELLOW "%s" COLOR_NONE ": 0x%08x " COLOR_DYELLOW "   ||"  COLOR_NONE "\n", regs[i], gpr(i));
  }
  printf(COLOR_DYELLOW "\n===== CSRS =====\n" COLOR_NONE);
  for (int i = 0; i < ARRLEN(csrs); i++) {
    printf(COLOR_YELLOW "%s" COLOR_NONE ": 0x%08x " COLOR_DYELLOW "   ||"  COLOR_NONE "\n", csrs[i], dut->out_csr[i]);
  }
  printf("\n");
}

uint32_t isa_reg_str2val(const char *s, bool *success) {
  if(success == NULL) {
    assert(0 &&"success is NULL!");
    return 0;
  }

  if(s == NULL) {
    assert(0 && "reg_name is NULL!");
    *success = false;
    return 0;
  }

  for (int i = 0; i < ARRLEN(regs); i++) {
    if(strcmp(regs[i], s) == 0) {
      *success = true;
      return gpr(i);
    }
  }

  if(strcmp("pc", s) == 0) {
    *success = true;
    return dut->out_pc;
  }
  
  printf("Unknown regs.\n");
  *success = false;
  return 0;
}