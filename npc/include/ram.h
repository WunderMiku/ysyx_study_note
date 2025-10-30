#ifndef __RAM_H__
#define __RAM_H__ 

extern "C" int pmem_read(int raddr);

extern "C" void pmem_write(int waddr, int wdata, char wmask);

#endif