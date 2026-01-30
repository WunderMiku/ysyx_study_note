#ifndef __DIFFTEST_H__
#define __DIFFTEST_H__

#include "config.h"
#include "npc.h"
#include <stdint.h>
#include <stdbool.h>

/* 数据传输方向枚举 */
enum {
  DIFFTEST_TO_DUT,  /* 从参考实现(REF)传输到待测设计(DUT) */
  DIFFTEST_TO_REF   /* 从待测设计(DUT)传输到参考实现(REF) */
};

/*
 * difftest寄存器上下文结构
 * 用于在NEMU和Spike之间传输寄存器状态
 */
typedef struct {
  uint32_t gpr[RISCV_GPR_NUM];  /* 通用寄存器 */
  uint32_t csr[CSR_COUNT];      /* 控制状态寄存器 */
  uint32_t pc;                  /* 程序计数器 */
} diff_context_t;

void init_difftest(char *ref_so_file, long img_size, int port);
static void checkregs(CpuState *ref, uint32_t pc, uint32_t before_pc);
bool isa_difftest_checkregs(CpuState *ref_r, uint32_t pc, uint32_t before_pc);
void difftest_step(uint32_t pc, uint32_t before_pc);

extern void (*ref_read_reg)();
extern uint32_t (*ref_read_memory)(uint32_t addr, size_t len);

#endif