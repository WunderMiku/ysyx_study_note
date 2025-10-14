/***************************************************************************************
* Copyright (c) 2014-2024 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <isa.h>
#include <cpu/difftest.h>
#include "../local-include/reg.h"

bool isa_difftest_checkregs(CPU_state *ref_r, vaddr_t pc) {
  for(int i = 0; i < 32; i ++) {
    if (ref_r->gpr[i] != gpr(i)) {
      printf("Difftest failed at pc = " FMT_WORD "\n", pc);
      printf("reg %d: ref " FMT_WORD ", dut " FMT_WORD "\n",
          i, ref_r->gpr[i], gpr(i));
      return false;
    }
  }

  if(ref_r->pc != pc) {
      printf("Difftest failed at pc = " FMT_WORD "\n", pc);
      printf("pc: ref " FMT_WORD ", dut " FMT_WORD "\n",
          ref_r->pc, pc);
      return false;
    }

  return true;
}

void isa_difftest_attach() {
}
