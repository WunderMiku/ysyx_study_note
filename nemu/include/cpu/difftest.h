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

#ifndef __CPU_DIFFTEST_H__
#define __CPU_DIFFTEST_H__

#include <common.h>
#include <difftest-def.h>

/*
 * NEMU差分测试(Difftest)机制
 *
 * 核心思想：通过与参考实现(REF)逐指令比较来验证NEMU(DUT)的正确性
 * 工作流程：
 * 1. NEMU执行一条指令
 * 2. 参考实现执行相同指令
 * 3. 比较两者的寄存器状态
 * 4. 发现差异则终止执行并报告错误
 */

#ifdef CONFIG_DIFFTEST
/* 跳过参考实现检查：用于处理不确定行为的指令 */
void difftest_skip_ref();
/* 跳过DUT检查：用于处理QEMU指令打包情况 */
void difftest_skip_dut(int nr_ref, int nr_dut);
/* 设置补丁函数：用于特殊指令处理 */
void difftest_set_patch(void (*fn)(void *arg), void *arg);
/* 单步执行并检查：核心difftest函数，每条指令执行后调用 */
void difftest_step(vaddr_t pc, vaddr_t npc);
/* 断开与参考实现的连接 */
void difftest_detach();
/* 重新连接到参考实现 */
void difftest_attach();
#else
/* 当CONFIG_DIFFTEST未定义时，提供空实现 */
static inline void difftest_skip_ref() {}
static inline void difftest_skip_dut(int nr_ref, int nr_dut) {}
static inline void difftest_set_patch(void (*fn)(void *arg), void *arg) {}
static inline void difftest_step(vaddr_t pc, vaddr_t npc) {}
static inline void difftest_detach() {}
static inline void difftest_attach() {}
#endif

/* 参考实现函数指针 - 通过动态库加载 */
/* 内存复制函数：用于同步DUT和REF的内存状态 */
extern void (*ref_difftest_memcpy)(paddr_t addr, void *buf, size_t n, bool direction);
/* 寄存器复制函数：用于同步DUT和REF的寄存器状态 */
extern void (*ref_difftest_regcpy)(void *dut, bool direction);
/* 执行函数：让参考实现执行指定数量的指令 */
extern void (*ref_difftest_exec)(uint64_t n);
/* 中断触发函数：用于同步中断状态 */
extern void (*ref_difftest_raise_intr)(uint64_t NO);

/*
 * 寄存器比较函数
 * @param name 寄存器名称
 * @param pc   当前指令地址
 * @param ref  参考实现寄存器值
 * @param dut  NEMU寄存器值
 * @return     比较结果，true表示一致，false表示不一致
 */
static inline bool difftest_check_reg(const char *name, vaddr_t pc, word_t ref, word_t dut) {
  if (ref != dut) {
    Log("%s is different after executing instruction at pc = " FMT_WORD
        ", right = " FMT_WORD ", wrong = " FMT_WORD ", diff = " FMT_WORD,
        name, pc, ref, dut, ref ^ dut);
    return false;
  }
  return true;
}

#endif
