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

#include "mmu.h"
#include "sim.h"
#include "../../include/common.h"
#include <difftest-def.h>

/*
 * Spike参考实现的difftest接口
 * Spike是一个开源的RISC-V ISA模拟器，用作参考实现
 */

/* RISC-V通用寄存器数量定义 */
#define NR_GPR MUXDEF(CONFIG_RVE, 16, 32)

static std::vector<std::pair<reg_t, abstract_device_t*>> difftest_plugin_devices;
static std::vector<std::string> difftest_htif_args;
static std::vector<std::pair<reg_t, mem_t*>> difftest_mem(
    1, std::make_pair(reg_t(DRAM_BASE), new mem_t(CONFIG_MSIZE)));
static debug_module_config_t difftest_dm_config = {
  .progbufsize = 2,
  .max_sba_data_width = 0,
  .require_authentication = false,
  .abstract_rti = 0,
  .support_hasel = true,
  .support_abstract_csr_access = true,
  .support_abstract_fpr_access = true,
  .support_haltgroups = true,
  .support_impebreak = true
};

/* Spike模拟器全局变量 */
static sim_t* s = NULL;       /* Spike模拟器实例 */
static processor_t *p = NULL; /* 处理器实例 */
static state_t *state = NULL; /* 处理器状态 */

/*
 * Spike模拟器difftest初始化
 * @param port 通信端口（当前未使用）
 */
void sim_t::diff_init(int port) {
  p = get_core("0");      /* 获取第一个处理器核心 */
  state = p->get_state();   /* 获取处理器状态 */
}

/*
 * Spike模拟器单步执行
 * @param n 执行的指令数量
 */
void sim_t::diff_step(uint64_t n) {
  step(n);
}

/*
 * 从Spike获取寄存器状态到difftest上下文
 * @param diff_context 寄存器上下文指针
 */
void sim_t::diff_get_regs(void* diff_context) {
  diff_context_t* ctx = (diff_context_t*)diff_context;
  for (int i = 0; i < NR_GPR; i++) {
    ctx->gpr[i] = state->XPR[i];  /* 复制通用寄存器 */
  }
  ctx->pc = state->pc;            /* 复制程序计数器 */
}

/*
 * 从difftest上下文设置Spike寄存器状态
 * @param diff_context 寄存器上下文指针
 */
void sim_t::diff_set_regs(void* diff_context) {
  diff_context_t* ctx = (diff_context_t*)diff_context;
  for (int i = 0; i < NR_GPR; i++) {
    state->XPR.write(i, (sword_t)ctx->gpr[i]);  /* 设置通用寄存器 */
  }
  state->pc = ctx->pc;                          /* 设置程序计数器 */
}

/*
 * 内存复制函数：从源地址复制数据到Spike内存
 * @param dest 目标地址（Spike内存）
 * @param src  源数据指针
 * @param n    复制字节数
 */
void sim_t::diff_memcpy(reg_t dest, void* src, size_t n) {
  mmu_t* mmu = p->get_mmu();
  for (size_t i = 0; i < n; i++) {
    mmu->store<uint8_t>(dest+i, *((uint8_t*)src+i));  /* 逐字节写入内存 */
  }
}

extern "C" {

/*
 * difftest内存复制接口 - 供NEMU调用
 * 将数据从NEMU复制到Spike内存
 */
__EXPORT void difftest_memcpy(paddr_t addr, void *buf, size_t n, bool direction) {
  if (direction == DIFFTEST_TO_REF) {
    s->diff_memcpy(addr, buf, n);  /* 复制到参考实现 */
  } else {
    assert(0);  /* 当前不支持从REF到DUT的内存复制 */
  }
}

/*
 * difftest寄存器复制接口 - 供NEMU调用
 * 在NEMU和Spike之间同步寄存器状态
 */
__EXPORT void difftest_regcpy(void* dut, bool direction) {
  if (direction == DIFFTEST_TO_REF) {
    s->diff_set_regs(dut);  /* 设置Spike寄存器 */
  } else {
    s->diff_get_regs(dut);  /* 获取Spike寄存器 */
  }
}

/*
 * difftest执行接口 - 供NEMU调用
 * 让Spike执行指定数量的指令
 */
__EXPORT void difftest_exec(uint64_t n) {
  s->diff_step(n);
}

/*
 * difftest初始化接口 - 供NEMU调用
 * 创建并初始化Spike模拟器实例
 * @param port 通信端口
 */
__EXPORT void difftest_init(int port) {
  difftest_htif_args.push_back("");

  // 根据配置构建ISA字符串（如"RV32IMA"）
  const char *isa = "RV" MUXDEF(CONFIG_RV64, "64", "32") MUXDEF(CONFIG_RVE, "E", "I") "MAFDC";

  // 配置Spike模拟器参数
  cfg_t cfg(/*default_initrd_bounds=*/std::make_pair((reg_t)0, (reg_t)0),
            /*default_bootargs=*/nullptr,
            /*default_isa=*/isa,
            /*default_priv=*/DEFAULT_PRIV,
            /*default_varch=*/DEFAULT_VARCH,
            /*default_misaligned=*/false,
            /*default_endianness*/endianness_little,
            /*default_pmpregions=*/16,
            /*default_mem_layout=*/std::vector<mem_cfg_t>(),
            /*default_hartids=*/std::vector<size_t>(1),
            /*default_real_time_clint=*/false,
            /*default_trigger_count=*/4);

  // 创建Spike模拟器实例
  s = new sim_t(&cfg, false,
      difftest_mem, difftest_plugin_devices, difftest_htif_args,
      difftest_dm_config, nullptr, false, NULL,
      false,
      NULL,
      true);

  // 初始化difftest相关功能
  s->diff_init(port);
}

__EXPORT void difftest_raise_intr(uint64_t NO) {
  trap_t t(NO);
  p->take_trap_public(t, state->pc);
}

}
