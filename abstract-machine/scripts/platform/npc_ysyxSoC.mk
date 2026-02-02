AM_SRCS := riscv/npc_ysyxSoC/start.S \
           riscv/npc_ysyxSoC/trm.c \
           riscv/npc_ysyxSoC/ioe.c \
           riscv/npc_ysyxSoC/timer.c \
           riscv/npc_ysyxSoC/input.c \
           riscv/npc_ysyxSoC/cte.c \
           riscv/npc_ysyxSoC/trap.S \
           riscv/npc_ysyxSoC/uart.c \
           riscv/npc_ysyxSoC/bootloader.c \
           platform/dummy/vme.c \
           platform/dummy/mpe.c

CFLAGS    += -fdata-sections -ffunction-sections
CFLAGS    += -I$(AM_HOME)/am/src/riscv/npc_ysyxSoC/include
LDSCRIPTS += $(AM_HOME)/scripts/SoClinker.ld
LDFLAGS   += --defsym=_prom_start=0x20000000 --defsym=_entry_offset=0x0 --defsym=_pram_start=0x0F000000
LDFLAGS   += --gc-sections -e _start

MAINARGS_MAX_LEN = 64
MAINARGS_PLACEHOLDER = the_insert-arg_rule_in_Makefile_will_insert_mainargs_here
CFLAGS += -DMAINARGS_MAX_LEN=$(MAINARGS_MAX_LEN) -DMAINARGS_PLACEHOLDER=$(MAINARGS_PLACEHOLDER)

insert-arg: image
	@python $(AM_HOME)/tools/insert-arg.py $(IMAGE).bin $(MAINARGS_MAX_LEN) $(MAINARGS_PLACEHOLDER) "$(mainargs)"

image: image-dep
	@$(OBJDUMP) -d $(IMAGE).elf > $(IMAGE).txt
	@echo + OBJCOPY "->" $(IMAGE_REL).bin
	@$(OBJCOPY) -S --set-section-flags .bss=alloc,contents -O binary $(IMAGE).elf $(IMAGE).bin

run: insert-arg
	$(MAKE) -C $(NPC_SOC_HOME) SIM_ARGS="$(IMAGE).bin" ELF_PATH="$(IMAGE).elf" BATMODE=true sim

.PHONY: insert-arg
