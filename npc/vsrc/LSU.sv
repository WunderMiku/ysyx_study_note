module LSU ( 
	// 写端口控制信号
	input  ram_we_EX,
	input  [31:0] ram_write_data_EX,
	input  [31:0] ram_write_addr_EX,
	input  [3:0]  ram_write_mask_EX,

	// 读端口控制信号
	input  ram_re_EX,
	input  [31:0] ram_read_addr_EX,

	// LSU 输出
	output ram_we,
	output [31:0] ram_write_data,
	output [31:0] ram_write_addr,
	output [3:0]  ram_write_mask,

	output ram_re,
	output [31:0] ram_read_addr
);

	assign ram_we = ram_we_EX;
	assign ram_write_data = ram_write_data_EX;
	assign ram_write_addr = ram_write_addr_EX;
	assign ram_write_mask = ram_write_mask_EX;

	assign ram_re = ram_re_EX;
	assign ram_read_addr = ram_read_addr_EX;

endmodule