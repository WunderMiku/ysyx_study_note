#ifndef __RAM_H__
#define __RAM_H__ 

#include <stdint.h>
extern "C" int pmem_read(int raddr, int len);

extern "C" void pmem_write(int waddr, int wdata, char wmask);

int paddr_read(int raddr, int len);
uint32_t fetchInst(uint32_t pc);

extern "C" void flash_read(int32_t addr, int32_t *data);
extern "C" void mrom_read(int32_t addr, int32_t *data);
#endif