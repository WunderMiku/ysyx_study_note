module ysyx_25090244_LSU ( 
	// 写端口控制信号
	input  ram_we_EX,
	input  [31:0] ram_write_data_EX,
	input  [31:0] ram_write_addr_EX,
	input  [3:0]  ram_write_mask_EX,

	// 读端口控制信号
	input  ram_re_EX,
	input  [31:0] ram_read_addr_EX,

	// LSU 输出
	output [31:0] ram_read_data,

	// 临时辅助信号（因为现在MEM读写端口实际是Top的端口实现的）
	// 写端口
	output 			    ram_we_Top,
	output [31:0] 	ram_write_data_Top,
	output [31:0] 	ram_write_addr_Top,
	output [3:0]  	ram_write_mask_Top,

	// 读端口
	output 			    ram_re_Top,
	output [31:0] 	ram_read_addr_Top,
	input  [31:0] 	ram_read_data_Top
);

// TODO : LSU 逻辑实现

	

endmodule