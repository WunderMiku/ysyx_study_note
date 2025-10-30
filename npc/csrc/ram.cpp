#include "npc.h"
#include "device.h"
#include <cassert>
extern "C" int pmem_read(int raddr) {
	if(raddr < MEM_BASE || raddr >= MEM_BASE + MEM_SIZE) {
		return deviceRead(raddr);
	}
	uint32_t vaddr = raddr - MEM_BASE;
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
	M[vaddr >> 2] = wdata_;
}