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

#include <SDL2/SDL_keycode.h>
#include <isa.h>
#include <cpu/cpu.h>
#include <difftest-def.h>
#include <memory/paddr.h>
#include <stdint.h>

void diff_memcpy(paddr_t dest, void* src, size_t n);
__EXPORT void difftest_memcpy(paddr_t addr, void *buf, size_t n, bool direction) {
  if(direction == DIFFTEST_TO_REF) {
    diff_memcpy(addr, buf, n);
  } else {
    assert(0);
  }
}

__EXPORT void difftest_regcpy(void *dut, bool direction) {
  diff_context_t *dut_context = (diff_context_t *)dut;
  if(direction == DIFFTEST_TO_REF) {
    for(int i = 0; i < RISCV_GPR_NUM; i++) {
      // dut_context->gpr[i] = cpu.gpr[i];
      cpu.gpr[i] = dut_context->gpr[i];
    }
    cpu.pc = dut_context->pc;
    // dut_context->pc = cpu.pc;
  } else {
    for(int i = 0; i < RISCV_GPR_NUM; i++) {
      // cpu.gpr[i] = dut_context->gpr[i];
      dut_context->gpr[i] = cpu.gpr[i];
    }
    // cpu.pc = dut_context->pc;
    dut_context->pc = cpu.pc;
  }
}

__EXPORT void difftest_exec(uint64_t n) {
  cpu_exec(n);
}

__EXPORT void difftest_raise_intr(word_t NO) {
  assert(0);
}

__EXPORT void difftest_init(int port) {
  void init_mem();
  init_mem();
  /* Perform ISA dependent initialization. */
  init_isa();
}

void diff_memcpy(paddr_t dest, void* src, size_t n) { 
	Assert(src != NULL, "memory src is NULL!");
	for(int i = 0; i < n; i++) {
		paddr_write(dest + i, 1, ((uint8_t*)src)[i]);
	}
}

__EXPORT uint32_t read_memory(uint32_t addr, int len) {
  return paddr_read(addr, len);
}

__EXPORT void read_reg() {
  isa_reg_display();
}