ifeq ($(VERILATOR_ROOT),)
VERILATOR = verilator
else
export VERILATOR_ROOT
VERILATOR = $(VERILATOR_ROOT)/bin/verilator
endif

CSRCS := $(wildcard csrc/*.cpp)
VSRCS := $(wildcard vsrc/*.v)

all: sim
	

sim:
	$(call git_commit, "sim RTL") # DO NOT REMOVE THIS LINE!!!
	@echo "-- Running Verilator --"
	$(VERILATOR) --cc --exe --build -j 0 -Wall $(CSRCS) $(VSRCS)

clean:
	-rm -rf obj_dir logs *.log *.dmp *.vpd coverage.dat core

include ../Makefile

