#include "stdint.h"
#include "npc_ysyxSoC.h"
#include <stdint.h>

#define SPI_BASE 0x10001000
#define SPI_Rx   (SPI_BASE + 0x00)
#define SPI_Tx   (SPI_BASE + 0x00)

#define SPI_Ctrl (SPI_BASE + 0x10)
#define SPI_DIV  (SPI_BASE + 0x14)
#define SPI_SS   (SPI_BASE + 0x18)

// uint32_t flash_read(uint32_t addr) {
// 	uint8_t send_data = 0xFE;

// 	outl(SPI_Rx, (uint32_t) send_data);
// 	outl(SPI_SS, 0x00000080);
// 	outl(SPI_DIV, 0x00000015);
// 	outl(SPI_Ctrl, 0x2b10);

// 	while(inl(SPI_Ctrl) & 0x100){} // wait

// 	uint8_t receive_data = (uint8_t) ((inl(SPI_Rx) & 0x0000ff00) >> 8);
// 	if(receive_data == 0x7F) {
// 		return 0;
// 	} else {
// 		return 1;
// 	}
// }