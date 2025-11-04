#include "difftest.h"
#include "config.h"
#include "npc.h"
#include "reg.h"
#include <assert.h>
#include <dlfcn.h>
#include <stdint.h>
#include <stdio.h>
#include "reg.h"

bool isa_difftest_checkregs(CpuState *ref_r, uint32_t pc);

void (*ref_difftest_memcpy)(uint32_t addr, void *buf, size_t n, bool direction) = NULL;
void (*ref_difftest_regcpy)(void *dut, bool direction) = NULL;
void (*ref_difftest_exec)(uint64_t n) = NULL;
void (*ref_difftest_raise_intr)(uint64_t NO) = NULL;
void (*ref_read_reg)() = NULL;
uint32_t (*ref_read_memory)(uint32_t addr, size_t len) = NULL;

void init_difftest(char *ref_so_file, long img_size, int port) {
	assert(ref_so_file != NULL);
	// 1. 动态加载参考实现库
  void *handle;
  handle = dlopen(ref_so_file, RTLD_LAZY);
  assert(handle);

  // 2. 获取参考实现函数指针
  ref_difftest_memcpy = (void (*)(uint32_t, void *, size_t, bool))dlsym(handle, "difftest_memcpy");
  assert(ref_difftest_memcpy);

  ref_difftest_regcpy = (void (*)(void *dut, bool direction))dlsym(handle, "difftest_regcpy");
  assert(ref_difftest_regcpy);

  ref_difftest_exec = (void(*)(uint64_t n))dlsym(handle, "difftest_exec");
  assert(ref_difftest_exec);

  ref_difftest_raise_intr = (void(*)(uint64_t NO))dlsym(handle, "difftest_raise_intr");
  assert(ref_difftest_raise_intr);

  ref_read_memory = (uint32_t(*)(uint32_t addr, size_t len))dlsym(handle, "read_memory");
  assert(ref_read_memory);

  ref_read_reg = (void (*)())dlsym(handle, "read_reg");
  assert(ref_read_reg);

  void (*ref_difftest_init)(int) = (void(*)(int))dlsym(handle, "difftest_init");
  assert(ref_difftest_init);

  printf("Differential testing: %s\n", COLOR_GREEN "ON");

  // 3. 初始化参考实现
  ref_difftest_init(port);

  // 4. 同步初始状态：内存和寄存器
	// for(int i = 0; i < 32; i ++) {printf("reg %d: ref 0x%08x\n", i, cpu.gpr[i]);}
  ref_difftest_memcpy(MEM_BASE, (void *)M, img_size, DIFFTEST_TO_REF);
  ref_difftest_regcpy(&cpu, DIFFTEST_TO_REF);
	
}

/*
 * 检查寄存器状态
 * @param ref 参考实现的寄存器状态
 * @param pc  当前指令地址
 */
static void checkregs(CpuState *ref, uint32_t pc, uint32_t before_pc) {
  if (!isa_difftest_checkregs(ref, pc, before_pc)) {
    // 寄存器状态不一致，终止执行
    npcState.state = NPC_ABORT;
    npcState.halt_pc = before_pc;
    isa_reg_display();  // 显示寄存器状态用于调试
  }
}

bool isa_difftest_checkregs(CpuState *ref_r, uint32_t pc, uint32_t before_pc) {
  // 1. 检查32个通用寄存器
  for(int i = 0; i < 32; i ++) {
    if (ref_r->gpr[i] != gpr(i)) {
      printf("Difftest failed at pc = 0x%08x\n", before_pc);
      printf("reg: " COLOR_RED "%s" COLOR_NONE ": ref_value: " COLOR_GREEN "0x%08x"  COLOR_NONE ", dut_value: " COLOR_RED "0x%08x" COLOR_NONE "\n",
          regs[i], ref_r->gpr[i], gpr(i));
      return false;
    }
  }

  // 2. 检查程序计数器(PC)
  if(ref_r->pc != pc) {
      printf("Difftest failed at pc = 0x%08x\n", before_pc);
      printf("pc: ref 0x%08x, dut 0x%08x\n",
          ref_r->pc, pc);
      return false;
    }

  return true;
}

/*
 * 单步执行并检查 - difftest核心函数
 * 每条指令执行后被调用，用于验证指令执行结果的正确性
 *
 * @param pc  当前指令地址
 * @param npc 下一条指令地址
 */
void difftest_step(uint32_t pc, uint32_t before_pc) {
  CpuState ref_r;

  ref_difftest_exec(1);

  // 获取参考实现的寄存器状态
  ref_difftest_regcpy(&ref_r, DIFFTEST_TO_DUT);

  // 比较寄存器状态
  checkregs(&ref_r, pc, before_pc);
}