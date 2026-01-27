include $(AM_HOME)/scripts/isa/riscv.mk
include $(AM_HOME)/scripts/platform/npc_ysyxSoC.mk
COMMON_CFLAGS += -march=rv32e_zicsr -mabi=ilp32e  # overwrite
LDFLAGS       += -melf32lriscv                    # overwrite

AM_SRCS += riscv/npc_ysyxSoC/libgcc/div.S \
           riscv/npc_ysyxSoC/libgcc/muldi3.S \
           riscv/npc_ysyxSoC/libgcc/multi3.c \
           riscv/npc_ysyxSoC/libgcc/ashldi3.c \
           riscv/npc_ysyxSoC/libgcc/unused.c
