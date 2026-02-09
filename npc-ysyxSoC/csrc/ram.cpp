#include "config.h"
#include "debug.h"
#include "ram.h"
#include "npc.h"
#include "device.h"
#include <cassert>
#include <cstdint>


static uint32_t get_mask_data(int paddr, int len) {
	switch (len) {
    case 1: return (uint8_t)  M[paddr];
    case 2: return (uint16_t) M[paddr];
    case 4: return (uint32_t) M[paddr];
    default: assert(0);
  }
}

extern "C" int pmem_read(int raddr, int len) {
	if(raddr < MEM_BASE || raddr >= MEM_BASE + MEM_SIZE) {
		return deviceRead(raddr);
	}

	uint32_t paddr = (raddr - MEM_BASE) >> 2;
	uint32_t data = M[paddr];

	data = get_mask_data(paddr, len);
#ifdef Mtrace_enable
	printf(COLOR_CYAN "[pmem_read] " COLOR_NONE " read [%d bit(s)] " COLOR_YELLOW "0x%08x" COLOR_NONE " from addr " COLOR_GREEN "0x%08x" COLOR_NONE " at pc = 0x%08x\n", len, data, paddr, cpu.pc);
#endif
	return data;
}
extern "C" void pmem_write(int waddr, int wdata, char wmask) {
	if((waddr < MEM_BASE || waddr >= MEM_BASE + MEM_SIZE) && waddr != SERIAL_PORT) {
		printf("写访问地址越界: %x\n", waddr);
		assert(0);
	}
	if(waddr == SERIAL_PORT) {
		putchar(wdata);
		fflush(stdout); // 刷新输出缓冲区
		return;
	}

	uint32_t vaddr = waddr - MEM_BASE;
  uint32_t current = M[vaddr >> 2];
  uint32_t byteMask = 0;

	// 将4位掩码扩展为32位掩码
	if (wmask & 0x1) byteMask |= 0x000000FF;
	if (wmask & 0x2) byteMask |= 0x0000FF00;  
	if (wmask & 0x4) byteMask |= 0x00FF0000;
	if (wmask & 0x8) byteMask |= 0xFF000000;

	uint32_t wdata_ = (current & ~byteMask) | (wdata & byteMask);
	M[vaddr >> 2] = wdata_;
#ifdef Mtrace_enable
        printf(COLOR_MAGENTA "[pmem_write]" COLOR_NONE
                             " [mask : %01d%01d%01d%01d] " COLOR_YELLOW
                             "0x%08x" COLOR_NONE " to addr " COLOR_GREEN
                             "0x%08x" COLOR_NONE " at pc = 0x%08x\n",
               (wmask & 0x1), ((wmask & 0x2) >> 1), ((wmask & 0x4) >> 2), ((wmask & 0x8) >> 3),
               wdata_, vaddr >> 2, cpu.pc);
#endif
}

int paddr_read(int raddr, int len) {
	if(!(len == 1 || len == 2 || len == 4)) {
		printf("length of data must be 1, 2 or 4!\n");
		assert(0);	
	}
	return pmem_read(raddr, len);
}

uint32_t fetchInst(uint32_t pc) {
	// return pmem_read(pc, 4);
	int32_t inst;
	if (pc >= MEM_BASE && pc < MEM_BASE + MEM_SIZE) {
		mrom_read(pc, &inst);
		return inst;
	} 
	else if (pc >= FLASH_BASE && pc < FLASH_BASE + FLASH_SIZE) {
		flash_read(pc - FLASH_BASE, &inst);
		return inst;
	}
	panic("pc is not in flash or mrom | pc = 0x%08x", pc);
}


// NEW MEMORY 
extern "C" void flash_read(int32_t addr, int32_t *data) {
	if(addr < 0 || addr >= FLASH_SIZE) {
		printf(COLOR_RED "Flash read out of range! | addr: 0x%08x\n" COLOR_NONE, addr);
		assert(0);
	}
	int32_t Flash_addr = (addr) >> 2;
	// printf("addr: %x\t Flash_addr: %x\n",addr, Flash_addr);
	*data = Flash[Flash_addr];
 }
extern "C" void mrom_read(int32_t addr, int32_t *data) { 
	if(addr < MEM_BASE || addr >= (MEM_BASE + MEM_SIZE)) {
		printf(COLOR_RED "Mrom read out of range! | addr: 0x%08x\n" COLOR_NONE, addr);
		assert(0);
	}
	int32_t M_addr = (addr - MROM_BASE) >> 2;
	*data = M[M_addr];
}
