#include <am.h>
#include <klib-macros.h>
#include <stdint.h>
#include "riscv/npc_ysyxSoC/include/npc_ysyxSoC.h"

#define LCR_ADDR SERIAL_PORT + 3
#define LSR_ADDR SERIAL_PORT + 5

// when DLAB == 1 
#define DIV_LSB_ADDR SERIAL_PORT
#define DIV_MSB_ADDR SERIAL_PORT + 1

__attribute__((section(".boot")))
void init_uart() {
	//allow access to divisor latch
	//uint8_t lcr = inb(LCR_ADDR);

	outb(LCR_ADDR, 0x83);

	outb(DIV_MSB_ADDR, 0x00);
	outb(DIV_LSB_ADDR, 0x01);
	// reset lcr_7
	outb(LCR_ADDR, 0x03);
}

bool uart_send_fifo_empty(){
	return (inb(LSR_ADDR) & (1 << 5));
}