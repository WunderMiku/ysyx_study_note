#include "macro.h"
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

void isa_reg_display() {
  for (int i = 0; i < ARRLEN(regs); i++) {
    printf("%s: 0x%08x\n", regs[i], gpr(i));
  }
}

// word_t isa_reg_str2val(const char *s, bool *success) {
//   if(success == NULL) {
//     assert(0 &&"success is NULL!");
//     return 0;
//   }

//   if(s == NULL) {
//     assert(0 && "reg_name is NULL!");
//     *success = false;
//     return 0;
//   }

//   for (int i = 0; i < ARRLEN(regs); i++) {
//     if(strcmp(regs[i], s) == 0) {
//       *success = true;
//       return gpr(i);
//     }
//   }

//   if(strcmp("pc", s) == 0) {
//     *success = true;
//     return cpu.pc;
//   }
  
//   Log("Unknown regs.");
//   *success = false;
//   return 0;
// }