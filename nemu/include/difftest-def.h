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

#ifndef __DIFFTEST_DEF_H__
#define __DIFFTEST_DEF_H__

#include "common.h"
#include <stdint.h>
#include <macro.h>
#include <generated/autoconf.h>

/* 导出符号属性，确保动态库中的函数可见 */
#define __EXPORT __attribute__((visibility("default")))

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
  word_t gpr[MUXDEF(CONFIG_RVE, 16, 32)];  /* 通用寄存器 */
  word_t pc;                               /* 程序计数器 */
  word_t csr[4];
} diff_context_t;

/* 不同ISA架构的寄存器大小定义 */
#if defined(CONFIG_ISA_x86)
/* x86架构：8个通用寄存器 + PC */
# define DIFFTEST_REG_SIZE (sizeof(uint32_t) * 9)
#elif defined(CONFIG_ISA_mips32)
/* MIPS32架构：32个通用寄存器 + 状态寄存器 + LO/HI + 异常相关寄存器 + PC */
# define DIFFTEST_REG_SIZE (sizeof(uint32_t) * 38)
#elif defined(CONFIG_ISA_riscv)
/* RISC-V架构：根据配置确定寄存器类型和数量 */
#define RISCV_GPR_TYPE MUXDEF(CONFIG_RV64, uint64_t, uint32_t)
#define RISCV_GPR_NUM  MUXDEF(CONFIG_RVE , 16, 32)
/* RISC-V：通用寄存器 + PC */
#define DIFFTEST_REG_SIZE (sizeof(RISCV_GPR_TYPE) * (RISCV_GPR_NUM + 1))
#elif defined(CONFIG_ISA_loongarch32r)
/* LoongArch32r架构：32个通用寄存器 + PC */
# define DIFFTEST_REG_SIZE (sizeof(uint32_t) * 33)
#else
# error Unsupport ISA
#endif

#endif
