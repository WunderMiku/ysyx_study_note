#include <stdio.h>

// 读取完整的64位 mcycle
static inline void read_mcycle(void) {
	unsigned int mcycle_lo, mcycle_hi;
	
	// 读取低32位
	asm volatile ("csrr %0, mcycle" : "=r"(mcycle_lo));
	// 读取高32位  
	asm volatile ("csrr %0, mcycleh" : "=r"(mcycle_hi));
	
	printf("mcycle: (hi)0x%08x (lo) 0x%08x\n", mcycle_hi, mcycle_lo);
}

