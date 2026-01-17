module ysyx_25090244_WBU (
	// EXU 输入
	input  [31:0] next_pc_EX,
	input         reg_we_EX,
	input  [4:0]  reg_addr_EX,
	input  [31:0] reg_data_EX,

	// WBU 输出
	output        reg_we,
	output [4:0]  reg_addr,
	output [31:0] reg_data,
	output [31:0] next_pc,

	output        pc_en,

	// Top 控制
	input         en
);
	// =========== 其他信号控制 ===========
	assign next_pc = next_pc_EX;
	assign reg_we_control = reg_we_EX;
	assign reg_addr = reg_addr_EX;
	assign reg_data = reg_data_EX;

	// 控制总线
	wire reg_we_control;
	assign pc_en = en;
	assign reg_we = reg_we_control & en;
	

endmodule