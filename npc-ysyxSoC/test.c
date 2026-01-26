#define UART_BASE 0x10000000
#define UART_TX   0x0
void _start() {
  *(volatile char *)(UART_BASE + UART_TX) = 'A';
  while (1);
}

