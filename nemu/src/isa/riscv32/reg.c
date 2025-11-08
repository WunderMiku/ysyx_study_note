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
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "local-include/reg.h"
#include "common.h"
#include "isa-def.h"
#include "macro.h"

const char *regs[] = {
  "$0", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
  "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
  "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
  "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"
};

const char *csrs_name[CSR_COUNT] = {
  "mepc", "mstatus", "mcause", "mtvec"
};


const uint32_t csrs_addr[CSR_COUNT] = {
  CSR_ADDR_MEPC, CSR_ADDR_MSTATUS, CSR_ADDR_MCAUSE, CSR_ADDR_MTVEC
};

int csr_addr_to_idx(word_t addr) { 
  for(int i = 0; i < CSR_COUNT; i++) {
    if(addr == csrs_addr[i]) {
      return i;
    }
  }
  panic("Unknown csr register | addr: 0x%08x", addr);
}

void isa_reg_display() {
  printf("===== GPRs =====\n");
  for (int i = 0; i < ARRLEN(regs); i++) {
    printf("%s: 0x%08x\n", regs[i], gpr(i));
  }
  printf("===== CSRs =====\n");
  for(int i = 0; i < ARRLEN(csrs_name); i++) {
    printf("%s: 0x%08x\n", csrs_name[i], csr(i));
  }
}

word_t isa_reg_str2val(const char *s, bool *success) {
  if(success == NULL) {
    Log("success is NULL!");
    return 0;
  }

  if(s == NULL) {
    Log("reg_name is NULL!");
    *success = false;
    return 0;
  }

  for (int i = 0; i < ARRLEN(regs); i++) {
    if(strcmp(regs[i], s) == 0) {
      *success = true;
      return gpr(i);
    }
  }

  for (int i = 0; i < ARRLEN(csrs_name); i++) {
    if(strcmp(csrs_name[i], s) == 0) {
      *success = true;
      return csr(i);
    }
  }

  if(strcmp("pc", s) == 0) {
    *success = true;
    return cpu.pc;
  }

  Log("Unknown regs.");
  *success = false;
  return 0;
}


