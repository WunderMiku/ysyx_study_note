module ysyx_25090244_RV32_regs (
	input clk,

	// 写端口
	input write_ena_in,
	input [4:0] write_addr,
	input [31:0] write_val,

	// 读端口1
	input [4:0] read_addr1,
	output [31:0] addr1_val,

	// 读端口2
	input [4:0] read_addr2,
	output [31:0] addr2_val,

	// A0寄存器输出
	output [31:0] A0_val,

	// 所有寄存器输出 (调试接口)
	output [31:0] reg_val [31:0]
);

	// 写使能控制
	wire write_ena = write_ena_in;

	reg [31:0] rv_regs [31:0];

	always @(posedge clk) begin 
		if(write_ena && (write_addr != 5'd0)) begin
			rv_regs[write_addr] <= write_val;
		end 
	end

	assign addr1_val = (read_addr1 != 5'b0) ? rv_regs[read_addr1] : 32'b0;
	assign addr2_val = (read_addr2 != 5'b0) ? rv_regs[read_addr2] : 32'b0;
	assign A0_val = rv_regs[10];

  // ================ 输出所有寄存器值 (sdb调试使用) ================
	genvar i;
	generate
		for (i = 0; i < 32; i = i + 1) begin : reg_assign
			assign reg_val[i] = rv_regs[i];
		end
	endgenerate

endmodule