#include "Vtop.h"
#include <bits/posix2_lim.h>
#include <cstdint>
#include <nvboard.h>
#include "verilated.h"
#include "verilated_fst_c.h"
#include "verilatedos.h"
#include "svdpi.h"


#define MAX_SIM_TIME 100

void nvboard_bind_all_pins(TOP_NAME* top);
// extern svBit ebreak_get();

vluint64_t sim_time = 0;
vluint32_t M[1024000] = 
{0};
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

	FILE *file = fopen(argv[2], "rb");
	if (file) {
		// 计算文件大小
		fseek(file, 0, SEEK_END);
		long file_size = ftell(file);
		rewind(file);
		
		// 确保不会超出M数组的大小
		size_t words_to_read = file_size / sizeof(uint32_t);
		if (words_to_read > sizeof(M) / sizeof(M[0])) {
			// debug("文件过大，无法加载到内存中,你需要%zu", words_to_read);
			printf("文件过大，无法加载到内存中,你需要%zu\n", words_to_read);
			return 1;
			// words_to_read = sizeof(M) / sizeof(M[0]);
		}
		
		// 读取数据到M数组
		size_t read_words = fread(M, sizeof(uint32_t), words_to_read, file);
		fclose(file);
		
		// debug("成功加载 %ld 字节 (%zu 个32位字) 到内存", read_words * sizeof(uint32_t), read_words);
		printf("成功加载 %ld 字节 (%zu 个32位字) 到内存\n", read_words * sizeof(uint32_t), read_words);
	} else {
		// debug("无法打开文件: %s", args);
		printf("无法打开文件: %s\n", argv[2]);
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
	//while(i < MAX_SIM_TIME) {
	while(1){
		// nvboard_update();
		single_cycle(dut.get());
		contextp->timeInc(1);
		tfp->dump(contextp->time());
		i++;
		// 检测是否结束
		svBit flag;
		dut->ebreak_get(&flag); 
		if(flag) {
			if(!dut->A0) {
				printf("EBREAK指令触发,程序结束: HIT GOOD TRAP, 总指令数:%d\n", i);
			} else {
				printf("EBREAK指令触发,程序结束: HIT BAD TRAP A0 = %08x, 总指令数:%d\n", dut->A0, i);
			}
			break;
		}
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
}

extern "C" int pmem_read(int raddr) {
	// printf("read: %x at M[%x]\n", M[raddr >> 2], raddr);
  return M[raddr >> 2];
}
extern "C" void pmem_write(int waddr, int wdata, char wmask) {
  uint32_t current = M[waddr >> 2];
  uint32_t byte_mask = 0;

	// 将4位掩码扩展为32位掩码
	if (wmask & 0x1) byte_mask |= 0x000000FF;
	if (wmask & 0x2) byte_mask |= 0x0000FF00;  
	if (wmask & 0x4) byte_mask |= 0x00FF0000;
	if (wmask & 0x8) byte_mask |= 0xFF000000;

	printf("Before: M[%x]: %x\n", waddr >> 2, current);

	M[waddr >> 2] = (current & ~byte_mask) | (wdata & byte_mask);
	printf("write: addr: %x, data: %x, mask: %b\n", waddr >> 2, wdata, wmask);
	printf("result: M[%x]: %x\n",waddr >> 2, M[waddr >> 2]);
}