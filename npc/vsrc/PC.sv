module ysyx_25090244_PC (
	input clk,
	input rst,

	input [31:0] next_pc,
	output reg [31:0] pc,

	simple_bus.slave bus_in
);

	always @(posedge clk or posedge rst) begin
		if (rst) begin
			pc <= 32'h80000000;
		end else begin
			if(bus_in.valid)
				pc <= next_pc;
			else
				pc <= pc;
		end
	end
endmodule