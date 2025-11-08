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
#include "isa-def.h"

/*
 * RISC-V 32位架构的difftest寄存器检查实现
 * 比较参考实现和NEMU的寄存器状态
 *
 * @param ref_r 参考实现的寄存器状态
 * @param pc    当前指令地址
 * @return      寄存器状态是否一致
 */
bool isa_difftest_checkregs(CPU_state *ref_r, vaddr_t pc) {
  // 1. 检查32个通用寄存器
  for(int i = 0; i < 32; i ++) {
    if (ref_r->gpr[i] != gpr(i)) {
      printf("Difftest failed at pc = " FMT_WORD "\n", pc);
      printf("reg %d: ref " FMT_WORD ", dut " FMT_WORD "\n",
          i, ref_r->gpr[i], gpr(i));
      return false;
    }
  }

  // 2. 检查程序计数器(PC)
  if(ref_r->pc != pc) {
      printf("Difftest failed at pc = " FMT_WORD "\n", pc);
      printf("pc: ref " FMT_WORD ", dut " FMT_WORD "\n",
          ref_r->pc, pc);
      return false;
    }

  for(int i = 0; i < CSR_COUNT; i++) {
    if(ref_r->csr[i] != csr(i)) {
      printf("Difftest failed at pc = " FMT_WORD "\n", pc);
      printf("csr %d: ref " FMT_WORD ", dut " FMT_WORD "\n",
          i, ref_r->csr[i], csr(i));
      return false;
    }
  }
  return true;
}

/*
 * RISC-V架构特定的difftest附加功能
 * 当前为空实现，可根据需要扩展
 */
void isa_difftest_attach() {
}
