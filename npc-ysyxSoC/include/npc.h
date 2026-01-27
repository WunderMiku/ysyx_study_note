#ifndef __NPC_H__
#define __NPC_H__

#include "VysyxSoCFull.h"
#include "verilatedos.h"
#include <stdint.h>

#define COLOR_RED    "\033[1;31m"
#define COLOR_GREEN  "\033[1;32m"
#define COLOR_YELLOW "\033[1;33m"
#define COLOR_BLUE   "\033[34m"
#define COLOR_BLACK   "\33[1;30m"
#define COLOR_MAGENTA "\33[1;35m"
#define COLOR_CYAN    "\33[1;36m"
#define COLOR_WHITE   "\33[1;37m"
#define COLOR_NONE   "\033[0m"

#define COLOR_SELECT(n) ((n) == 0 ? COLOR_RED : \
                         (n) == 1 ? COLOR_GREEN : \
                         (n) == 2 ? COLOR_YELLOW : \
                         (n) == 3 ? COLOR_BLUE : \
                         (n) == 4 ? COLOR_BLACK : \
                         (n) == 5 ? COLOR_MAGENTA : \
                         (n) == 6 ? COLOR_CYAN : COLOR_WHITE)

#define COLOR_BG_BLACK   "\33[1;40m"
#define COLOR_BG_RED     "\33[1;41m"
#define COLOR_BG_GREEN   "\33[1;42m"
#define COLOR_BG_YELLOW  "\33[1;43m"
#define COLOR_BG_BLUE    "\33[1;44m"
#define COLOR_BG_MAGENTA "\33[1;45m"
#define COLOR_BG_CYAN    "\33[1;46m"
#define COLOR_BG_WHITE   "\33[1;47m"
#define COLOR_BG_LYELLOW "\033[103m"
#define COLOR_BG_LGREEN  "\033[102m"

#define COLOR_MIKU "\033[38;2;39;197;187m"
#define COLOR_DYELLOW "\033[38;2;255;199;6m"

#define BOLD_TEXT "\033[1m"
#define ITALIC_TEXT "\033[3m"

#define MEM_SIZE 0x0FFF

// #include "config.h"

// typedef enum {
// 	NPC_RUNNING = 0,
// 	NPC_END,
// 	NPC_QUIT,
// 	NPC_STOP,
// 	NPC_ABORT
// } NpcStateType;

// typedef struct {
//   NpcStateType state;
// 	uint32_t inst, halt_pc;
// 	char instLog[256];
// } NpcState;

// typedef struct {
//   uint32_t gpr[RISCV_GPR_NUM];  /* 通用寄存器 */
//   uint32_t csr[CSR_COUNT];      /* 控制寄存器 */
//   uint32_t pc;                  /* 程序计数器 */
// } CpuState;
// ===== 全局对象（在 main.cpp 中定义）=====
extern vluint32_t* M;
extern std::unique_ptr<VerilatedContext> contextp;
extern VerilatedFstC*    tfp;
extern std::unique_ptr<VysyxSoCFull> dut;


// ===== 仿真初始化 =====
void verilatorInit(
    VerilatedFstC* tfp,
    VerilatedContext* contextp,
    VysyxSoCFull* dut,
    int argc,
    char** argv
);

// ===== CPU / 仿真控制接口 =====
void cpuExec(uint32_t n);
void execOnce();
void singleCycle(VysyxSoCFull* dut);
void reset(VysyxSoCFull* dut);
void sim_init(int argc, char** argv);

// ===== 工具函数 =====
int get_random(int n);
int checkEbreak();

#endif // NPC_H__