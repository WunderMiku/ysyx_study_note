#include <stdint.h>
#include <generated/autoconf.h>
#include <common.h>
#include <device/map.h>
void init_sram() {
	uint32_t *sram_base = (uint32_t *)new_space(CONFIG_SRAM_SIZE);
#ifdef CONFIG_HAS_PORT_IO
#else
  add_mmio_map("sram", CONFIG_SRAM_MMIO, sram_base, CONFIG_SRAM_SIZE, NULL);
#endif
}