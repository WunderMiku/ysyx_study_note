module RV32_regs (
	input clk,

	// 写端口
	input write_ena,
	input [4:0] write_addr,
	input [31:0] write_val,

	// 读端口1
	input [4:0] read_addr1,
	output [31:0] addr1_val,

	// 读端口2
	input [4:0] read_addr2,
	output [31:0] addr2_val,

	// A0寄存器输出
	output [31:0] A0_val
);

	reg [31:0] rv_regs [31:0];

	always @(posedge clk) begin 
		if(write_ena && (write_addr != 5'd0)) begin
			rv_regs[write_addr] <= write_val;
		end 
	end

	assign addr1_val = (read_addr1 != 5'b0) ? rv_regs[read_addr1] : 32'b0;
	assign addr2_val = (read_addr2 != 5'b0) ? rv_regs[read_addr2] : 32'b0;
	assign A0_val = rv_regs[10];

endmodule