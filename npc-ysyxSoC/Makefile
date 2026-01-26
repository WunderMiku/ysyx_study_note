TOPNAME = ysyxSoCFull
NXDC_FILES = constr/top.nxdc
INC_PATH ?= $(NPC_SOC_HOME)/include
SIM_ARGS ?= ./resource/default.bin
ELF_PATH ?= $(NPC_SOC_HOME)/resource/default.elf
BATMODE  ?= false

# rules for batch mode
ifeq ($(BATMODE), true)
override SIM_ARGS += --batch
else
override SIM_ARGS += --no-batch
endif

ifeq ($(VERILATOR_ROOT),)
VERILATOR = verilator
else
export VERILATOR_ROOT
VERILATOR = $(VERILATOR_ROOT)/bin/verilator
endif

VERILATOR_CFLAGS += -MMD --build -cc -trace-fst \
				-O3 --x-assign fast --x-initial fast --noassert -j 0

BUILD_DIR = ./build
OBJ_DIR = $(BUILD_DIR)/obj_dir
BIN = $(BUILD_DIR)/$(TOPNAME)

$(shell mkdir -p $(BUILD_DIR))

# constraint file
SRC_AUTO_BIND = $(abspath $(BUILD_DIR)/auto_bind.cpp)
$(SRC_AUTO_BIND): $(NXDC_FILES)
	python3 $(NVBOARD_HOME)/scripts/auto_pin_bind.py $^ $@

# project source
CSRCS = $(shell find $(abspath ./csrc) -name "*.c" -o -name "*.cc" -o -name "*.cpp")

VPKGS := $(shell find $(abspath ./vsrc) -regex ".*_pkg\.sv")
VSRCS := $(shell find $(abspath ./vsrc) \( -name "*.v" -o -name "*.sv" \) ! -regex ".*_pkg\.sv")


# rules for NVBoard
# CSRCS += $(SRC_AUTO_BIND)
# include $(NVBOARD_HOME)/scripts/nvboard.mk

# rules for verilator
INCFLAGS = $(addprefix -I, $(INC_PATH))
CXXFLAGS += $(INCFLAGS) -DTOP_NAME="\"V$(TOPNAME)\""

# rules for dependent .h files
HSRCS = $(shell find $(abspath ./include) -name "*.h")
$(CSRCS): $(HSRCS)
# update timestamp to force recompile
	@touch $@ 

# rules for ysyxSOC
VERILATOR_CFLAGS_SOC += +incdir+$(SOC_HOME)/perip/uart16550/rtl \
                        +incdir+$(SOC_HOME)/perip/spi/rtl \
												--timescale 1ns/1ns --no-timing

VSCRS_SOC := $(shell find $(SOC_HOME)/perip  -name "*.v") $(SOC_HOME)/build/ysyxSoCFull.v

# build rules
$(BIN): $(VPKGS) $(VSRCS) $(VSCRS_SOC) $(CSRCS) $(NVBOARD_ARCHIVE)
	@rm -rf $(OBJ_DIR)
	@echo "-- Running Verilator --"
	$(VERILATOR) $(VERILATOR_CFLAGS) $(VERILATOR_CFLAGS_SOC)\
		--top-module $(TOPNAME) $^ \
		$(addprefix -CFLAGS , $(CXXFLAGS)) $(addprefix -LDFLAGS , $(LDFLAGS)) \
		--Mdir $(OBJ_DIR) --exe -o $(abspath $(BIN))


all: sim
	
sim: $(BIN)
	$(call git_commit, "sim RTL") # DO NOT REMOVE THIS LINE!!!
	@if [ "$(BATMODE)" = "true" ]; then \
		echo "-- Running in bat mode --"; \
	fi
	@$^ +trace $(SIM_ARGS) $(ELF_PATH)

clean:
	-rm -rf obj_dir logs *.log *.dmp *.vpd coverage.dat core $(BUILD_DIR)

.PHONY: all sim clean



