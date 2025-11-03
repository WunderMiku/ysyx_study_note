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

#include <dlfcn.h>

#include <isa.h>
#include <cpu/cpu.h>
#include <memory/paddr.h>
#include <utils.h>
#include <difftest-def.h>

/*
 * DUT端(DUT: Design Under Test) - NEMU模拟器
 * 负责加载参考实现动态库，调用参考实现函数，并进行状态比较
 */

/* 参考实现函数指针 - 通过动态库加载获得 */
void (*ref_difftest_memcpy)(paddr_t addr, void *buf, size_t n, bool direction) = NULL;
void (*ref_difftest_regcpy)(void *dut, bool direction) = NULL;
void (*ref_difftest_exec)(uint64_t n) = NULL;
void (*ref_difftest_raise_intr)(uint64_t NO) = NULL;

#ifdef CONFIG_DIFFTEST

/* 跳过参考实现检查标志 - 用于处理不确定行为的指令 */
static bool is_skip_ref = false;
/* 跳过DUT检查的指令计数 - 用于处理QEMU指令打包 */
static int skip_dut_nr_inst = 0;

/*
 * 跳过参考实现检查
 * 用途：当遇到不确定行为的指令时，跳过REF的检查
 * 例如：某些指令在不同实现中可能有不同的行为
 */
void difftest_skip_ref() {
  is_skip_ref = true;
  // 如果这样的指令是QEMU指令打包的一部分，我们结束追赶QEMU PC的过程
  // 以保持最佳的一致性行为。
  // 注意：这仍然不完美：如果打包的指令已经写入了某些内存，
  // 而NEMU中的传入指令将加载该内存，我们会遇到假阴性。
  // 但这种情况很少见。
  skip_dut_nr_inst = 0;
}

/*
 * 跳过DUT检查
 * 用途：处理QEMU指令打包问题
 * 问题：有时让QEMU单步执行一次会执行多条指令
 * 解决方案：跳过检查直到NEMU的PC赶上QEMU的PC
 *
 * @param nr_ref 让REF先执行的指令数
 * @param nr_dut 期望DUT在多少条指令内赶上REF
 */
void difftest_skip_dut(int nr_ref, int nr_dut) {
  skip_dut_nr_inst += nr_dut;

  // 让参考实现先执行nr_ref条指令
  while (nr_ref -- > 0) {
    ref_difftest_exec(1);
  }
}

/*
 * 初始化difftest机制
 * @param ref_so_file 参考实现动态库路径
 * @param img_size    镜像文件大小
 * @param port        通信端口
 */
void init_difftest(char *ref_so_file, long img_size, int port) {
  assert(ref_so_file != NULL);

  // 1. 动态加载参考实现库
  void *handle;
  handle = dlopen(ref_so_file, RTLD_LAZY);
  assert(handle);

  // 2. 获取参考实现函数指针
  ref_difftest_memcpy = dlsym(handle, "difftest_memcpy");
  assert(ref_difftest_memcpy);

  ref_difftest_regcpy = dlsym(handle, "difftest_regcpy");
  assert(ref_difftest_regcpy);

  ref_difftest_exec = dlsym(handle, "difftest_exec");
  assert(ref_difftest_exec);

  ref_difftest_raise_intr = dlsym(handle, "difftest_raise_intr");
  assert(ref_difftest_raise_intr);

  void (*ref_difftest_init)(int) = dlsym(handle, "difftest_init");
  assert(ref_difftest_init);

  Log("Differential testing: %s", ANSI_FMT("ON", ANSI_FG_GREEN));
  Log("The result of every instruction will be compared with %s. "
      "This will help you a lot for debugging, but also significantly reduce the performance. "
      "If it is not necessary, you can turn it off in menuconfig.", ref_so_file);

  // 3. 初始化参考实现
  ref_difftest_init(port);

  // 4. 同步初始状态：内存和寄存器
  ref_difftest_memcpy(RESET_VECTOR, guest_to_host(RESET_VECTOR), img_size, DIFFTEST_TO_REF);
  ref_difftest_regcpy(&cpu, DIFFTEST_TO_REF);
}

/*
 * 检查寄存器状态
 * @param ref 参考实现的寄存器状态
 * @param pc  当前指令地址
 */
static void checkregs(CPU_state *ref, vaddr_t pc) {
  if (!isa_difftest_checkregs(ref, pc)) {
    // 寄存器状态不一致，终止执行
    nemu_state.state = NEMU_ABORT;
    nemu_state.halt_pc = pc;
    isa_reg_display();  // 显示寄存器状态用于调试
  }
}

/*
 * 单步执行并检查 - difftest核心函数
 * 每条指令执行后被调用，用于验证指令执行结果的正确性
 *
 * @param pc  当前指令地址
 * @param npc 下一条指令地址
 */
void difftest_step(vaddr_t pc, vaddr_t npc) {
  CPU_state ref_r;

  // 1. 处理跳过DUT检查的情况（QEMU指令打包）
  if (skip_dut_nr_inst > 0) {
    ref_difftest_regcpy(&ref_r, DIFFTEST_TO_DUT);
    if (ref_r.pc == npc) {
      // DUT的PC已经赶上REF，恢复正常检查
      skip_dut_nr_inst = 0;
      checkregs(&ref_r, npc);
      return;
    }
    skip_dut_nr_inst --;
    if (skip_dut_nr_inst == 0)
      panic("can not catch up with ref.pc = " FMT_WORD " at pc = " FMT_WORD, ref_r.pc, pc);
    return;
  }

  // 2. 处理跳过REF检查的情况（不确定行为指令）
  if (is_skip_ref) {
    // 跳过指令检查：直接将DUT的寄存器状态复制到REF
    ref_difftest_regcpy(&cpu, DIFFTEST_TO_REF);
    is_skip_ref = false;
    return;
  }

  // 3. 正常difftest流程
  // 3.1 让参考实现执行一条指令
  ref_difftest_exec(1);

  // 3.2 获取参考实现的寄存器状态
  ref_difftest_regcpy(&ref_r, DIFFTEST_TO_DUT);

  // 3.3 比较寄存器状态
  checkregs(&ref_r, npc); // 注意：这里使用npc而不是pc，因为指令执行后PC已经更新
}
#else
void init_difftest(char *ref_so_file, long img_size, int port) { }
#endif
