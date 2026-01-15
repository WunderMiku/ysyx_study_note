module RegisterFile(
	input clk,
	input [31:0] raddr,
	input re,

	input [31:0] waddr,
	input [31:0] wdata,
	input [3:0] wmask,
	input we,

	output logic[31:0] rdata
);

	logic [31:0] regfile [255:0]; // 256 x 32 bits register file
	// 读操作
	always_comb begin
		if (re) begin
			rdata = regfile[raddr[9:2]]; // 32-bit aligned addressing
		end else begin
			rdata = 32'b0;
		end
	end

	// 写操作
	wire [31:0] wmask_wide;
	assign wmask_wide = {
		{8{wmask[3]}},
		{8{wmask[2]}},
		{8{wmask[1]}},
		{8{wmask[0]}}
	};

	always_ff @(posedge clk) begin
		if (we) begin
			regfile[waddr[9:2]] <= (wdata & wmask_wide) | (regfile[waddr[9:2]] & ~wmask_wide);
		end
	end
endmodule

module RegisterFile_ROM(
	input clk,
	input [31:0] raddr,
	input re,

	output logic[31:0] rdata
);

	logic [31:0] regfile [255:0]; // 256 x 32 bits register file
	// 读操作
	always_comb begin
		if (re) begin
			rdata = regfile[raddr[9:2]]; // 32-bit aligned addressing
		end else begin
			rdata = 32'b0;
		end
	end
endmodule