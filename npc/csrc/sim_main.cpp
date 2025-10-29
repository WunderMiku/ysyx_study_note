#include "Vtop.h"
#include <bits/posix2_lim.h>
#include <cassert>
#include <cstdint>
#include <nvboard.h>
#include "verilated.h"
#include "verilated_fst_c.h"
#include "verilatedos.h"
#include "svdpi.h"
#include "../include/npc.h"


#define COLOR_RED   "\033[1;31m"
#define COLOR_GREEN "\033[1;32m"
#define COLOR_NONE  "\033[0m"

#define MAX_SIM_TIME 100

void nvboard_bind_all_pins(TOP_NAME* top);
// extern svBit ebreak_get();

vluint64_t sim_time = 0;
//vluint32_t M[644245095] = 
//{0};

#include <cstdlib>
#include <cstring>

// 使用动态分配
static vluint32_t* M = nullptr;

#define MEM_BASE 0x80000000
#define MEM_SIZE 0x8000000
void init_mem() {
    M = (vluint32_t*)calloc(MEM_SIZE, sizeof(vluint32_t));
    if (!M) {
        printf("Memory allocation failed!\n");
        exit(1);
    }
}

// {0x01400513, 0x010000e7, 0x00c000e7, 0x00100073, 0x00a50513, 0x00008067};

static void single_cycle(Vtop*);
static void reset(Vtop*);

// 存储器接口函数
extern "C" int pmem_read(int raddr);
extern "C" void pmem_write(int waddr, int wdata, char wmask);
int main(int argc, char** argv) {
	// for(int i = 2; i < argc; i++) {
	// 	printf("%s\n", argv[i]);
	// }
	init_mem();
	FILE *file = fopen(argv[2], "rb");
	if (file) {
		// 计算文件大小
		fseek(file, 0, SEEK_END);
		long file_size = ftell(file);
		rewind(file);
		
		// 确保不会超出M数组的大小
		size_t words_to_read = file_size / sizeof(uint32_t);
		if (words_to_read > MEM_SIZE) {
			// debug("文件过大，无法加载到内存中,你需要%zu", words_to_read);
			printf("文件过大，无法加载到内存中,你需要%zu\n", words_to_read);
			return 1;
			// words_to_read = sizeof(M) / sizeof(M[0]);
		}
		
		// 读取数据到M数组
		size_t read_words = fread(M, sizeof(uint32_t), words_to_read, file);
		fclose(file);
		
		// debug("成功加载 %ld 字节 (%zu 个32位字) 到内存", read_words * sizeof(uint32_t), read_words);
		printf(COLOR_GREEN "成功加载 %ld 字节 (%zu 个32位字) 到内存\n" COLOR_NONE , read_words * sizeof(uint32_t), read_words);
	} else {
		// debug("无法打开文件: %s", args);
		printf(COLOR_RED "无法打开文件: %s\n" COLOR_NONE , argv[2]);
		return 1;
	}

	Verilated::mkdir("logs"); // Create a directory for logs

	const std::unique_ptr<VerilatedContext> contextp{new VerilatedContext};
	const std::unique_ptr<Vtop> dut{new Vtop{contextp.get(), "TOP"}};

	contextp->debug(0);
	contextp->traceEverOn(true); // Enable tracing
	contextp->commandArgs(argc, argv);

	VerilatedFstC* tfp = new VerilatedFstC;

	dut->trace(tfp, 5);  // 设置跟踪深度
	tfp->open("logs/sim.fst");  // 打开并创建波形文件

	// 得到DPI函数导入需要的scope
	const svScope scope = svGetScopeFromName("TOP.top.uEXU");  
	assert(scope);
	svSetScope(scope);

	// nvboard_bind_all_pins(dut.get());
  // nvboard_init();

	reset(dut.get());
	contextp->timeInc(1);
	tfp->dump(contextp->time());
	// nvboard_update();
	int i = 0;
	// while(i < MAX_SIM_TIME) {
	while(1){
		// nvboard_update();

		// Ram 接口处理
		if(dut->ramRe) { // ram读
			//printf("ram读: M[%x]\n", dut->ramReadAddr);
			dut->ramReadData = pmem_read(dut->ramReadAddr);
		}

		if(dut->ramWe) { // ram写
			//printf("ram写: M[%x]\n", dut->ramWriteAddr);
			pmem_write(dut->ramWriteAddr, dut->ramWriteData, dut->ramWriteMask);
		}

		// 检测是否结束
		svBit flag;
		dut->ebreak_get(&flag); 
		if(flag) {
			if(!dut->A0) {
				printf("EBREAK指令触发,程序结束: " COLOR_GREEN "HIT GOOD TRAP" COLOR_NONE " 总指令数:%d\n", i);
			} else {
				printf("EBREAK指令触发,程序结束: " COLOR_RED "HIT BAD TRAP" COLOR_NONE " A0 = %08x, 总指令数:%d\n", dut->A0, i);
			}
			break;
		}
		// 电路步进
		single_cycle(dut.get());
		contextp->timeInc(1);
		tfp->dump(contextp->time());
		i++;
	}

	dut->final();
	tfp->close();
  // nvboard_quit();

	return 0;
}

static void single_cycle(Vtop* dut) {
  dut->clk = 0; dut->eval();
  dut->clk = 1; dut->eval();
	// printf("a0: 0x%08x\n", dut->A0);
}

static void reset(Vtop *dut) {
	dut->rst = 1;
	single_cycle(dut);
	dut->rst = 0;
	dut->eval();
	get_time();
}
extern "C" int pmem_read(int raddr) {
	if(raddr < MEM_BASE || raddr >= MEM_BASE + MEM_SIZE) {
		// printf("尝试设备访问: %x\n", raddr);
		return device_read(raddr);
		// printf("读访问地址越界: %x\n", raddr);
		// assert(0);
	}
	uint32_t vaddr = raddr - MEM_BASE;
	// printf("read: %x at M[%x]\n", M[vaddr >> 2], vaddr);
  return M[vaddr >> 2];
}
extern "C" void pmem_write(int waddr, int wdata, char wmask) {
	if((waddr < MEM_BASE || waddr >= MEM_BASE + MEM_SIZE) && waddr != SERIAL_PORT) {
		printf("写访问地址越界: %x\n", waddr);
		assert(0);
	}
	if(waddr == SERIAL_PORT) {
		putchar(wdata);
		return;
	}

	uint32_t vaddr = waddr - MEM_BASE;
  uint32_t current = M[vaddr >> 2];
  uint32_t byte_mask = 0;

	// 将4位掩码扩展为32位掩码
	if (wmask & 0x1) byte_mask |= 0x000000FF;
	if (wmask & 0x2) byte_mask |= 0x0000FF00;  
	if (wmask & 0x4) byte_mask |= 0x00FF0000;
	if (wmask & 0x8) byte_mask |= 0xFF000000;

	uint32_t wdata_ = (current & ~byte_mask) | (wdata & byte_mask);

	// printf("Before: M[%x]: %x\n", vaddr >> 2, current);
	

	M[vaddr >> 2] = wdata_;
	// printf("write: addr: %x, data: %x, mask: %b\n", vaddr >> 2, wdata, wmask);
	// printf("result: M[%x]: %x\n",vaddr >> 2, M[vaddr >> 2]);
}