#include "npc.h"
#include "device.h"
#include <cassert>
#include <cstdint>


extern "C" int pmem_read(int raddr, int len) {
	if(raddr < MEM_BASE || raddr >= MEM_BASE + MEM_SIZE) {
		return deviceRead(raddr);
	}
	
	uint32_t paddr = (raddr - MEM_BASE) >> 2;

	switch (len) {
    case 1: return (uint8_t)  M[paddr];
    case 2: return (uint16_t) M[paddr];
    case 4: return (uint32_t) M[paddr];
    default: assert(0);
  }
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
  uint32_t byteMask = 0;

	// 将4位掩码扩展为32位掩码
	if (wmask & 0x1) byteMask |= 0x000000FF;
	if (wmask & 0x2) byteMask |= 0x0000FF00;  
	if (wmask & 0x4) byteMask |= 0x00FF0000;
	if (wmask & 0x8) byteMask |= 0xFF000000;

	uint32_t wdata_ = (current & ~byteMask) | (wdata & byteMask);
	M[vaddr >> 2] = wdata_;
}

int paddr_read(int raddr, int len) {
	if(!(len == 1 || len == 2 || len == 4)) {
		printf("length of data must be 1, 2 or 4!\n");
		assert(0);	
	}
	return pmem_read(raddr, len);
}
