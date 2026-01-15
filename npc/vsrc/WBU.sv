module ysyx_25090244_WBU (
	// EXU 输入
	input  [31:0] next_pc_EX,
	input         reg_we_EX,
	input  [4:0]  reg_addr_EX,
	input  [31:0] reg_data_EX,


	input  [11:0] csr_raddr_EX,

	input         csr_we_EX,
	input  [11:0] csr_waddr_EX,
	input  [31:0] csr_wdata_EX,

	input         csr_we1_EX,
	input  [11:0] csr_waddr1_EX,
	input  [31:0] csr_wdata1_EX,

	// WBU 输出
	output        reg_we,
	output [4:0]  reg_addr,
	output [31:0] reg_data,
	output [31:0] next_pc,

	output        csr_we,
	output [11:0] csr_raddr,

	output [11:0] csr_waddr,
	output [31:0] csr_wdata,

	output        csr_we1,
	output [11:0] csr_waddr1,
	output [31:0] csr_wdata1,

	simple_bus.slave bus_in,
	simple_bus.master bus_out
);
	// =========== 总线信号控制 ===========
	assign bus_out.valid = bus_in.valid;

	// =========== 其他信号控制 ===========
	assign next_pc = next_pc_EX;
	assign reg_we = reg_we_EX;
	assign reg_addr = reg_addr_EX;
	assign reg_data = reg_data_EX;

	assign csr_raddr = csr_raddr_EX;

	assign csr_we = csr_we_EX;
	assign csr_waddr = csr_waddr_EX;
	assign csr_wdata = csr_wdata_EX;

	assign csr_we1 = csr_we1_EX;
	assign csr_waddr1 = csr_waddr1_EX;
	assign csr_wdata1 = csr_wdata1_EX;

endmodule