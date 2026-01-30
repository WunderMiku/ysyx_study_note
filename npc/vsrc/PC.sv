module ysyx_25090244_PC (
	input clk,
	input rst,

	input [31:0] next_pc,
	output reg [31:0] pc,
	input update_en
);

	// PC 更新逻辑
	always @(posedge clk or posedge rst) begin
		if (rst) begin
			pc <= 32'h20000000;
		end else begin
			if(update_en)
				pc <= next_pc;
			 else
			 	pc <= pc;
		end
	end
endmodule