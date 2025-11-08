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

// ecall 未调用这个函数，改为直接在inst执行
word_t isa_raise_intr(word_t NO, vaddr_t epc) {
  cpu.csr[0] = epc;     // set mepc
  cpu.csr[2] = NO;      // set mcause
  cpu.pc = cpu.csr[3];  // jump to mtvec
  return 0;
}

word_t isa_query_intr() {
  return INTR_EMPTY;
}
