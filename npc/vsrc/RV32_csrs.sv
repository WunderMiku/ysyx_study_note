module RV32_csrs (
	input clk,
	input rst,

	// csr 读写接口
	input we,
	input [11:0] waddr,
	input [31:0] wdata,

	input [11:0] raddr,
	output reg [31:0] rdata,

 	// 附加写接口
	input we1,
	input [11:0] waddr1,
	input [31:0] wdata1,

	// 调试接口
	output reg [31:0] out_csr [3:0]
);

// mcycle 
	reg [63:0] mcycle_reg;
// mvendorid
	reg [31:0] mvendorid = 32'h79737978;
// marchid
	reg [31:0] marchid = 32'h17ED8C4;
// mepc
	reg [31:0] mepc;
//mstatus
	reg [31:0] mstatus;
// mcause
	reg [31:0] mcause;
// mtvec
	reg [31:0] mtvec;

  // 调试接口
	assign out_csr[0] = mepc[31:0];
	assign out_csr[1] = mstatus[31:0];
	assign out_csr[2] = mcause[31:0];
	assign out_csr[3] = mtvec[31:0];

	always @(posedge clk or posedge rst) begin
		if(rst) begin
			mstatus <= 32'h1800;
		end else begin
			if(we) begin
				case(waddr)
					12'hB00: mcycle_reg <= {mcycle_reg[63:32] ,wdata};
					12'hB80: mcycle_reg <= {wdata, mcycle_reg[31:0]};
					12'h341: mepc <= wdata;
					12'h300: mstatus <= wdata;
					12'h342: mcause <= wdata;
					12'h305: mtvec <= wdata;
					default: begin
						mcycle_reg <= mcycle_reg + 1;
					end
				endcase
			end else begin
				mcycle_reg <= mcycle_reg + 1;
			end
		end

		if(we1) begin
			case(waddr1)
				12'h341: mepc <= wdata1;
				12'h300: mstatus <= wdata1;
				12'h342: mcause <= wdata1;
				12'h305: mtvec <= wdata1;
				default: begin
				end
			endcase
		end
	end

	always @(*) begin
		case(raddr)
			12'hB00: rdata = mcycle_reg[31:0];
			12'hB80: rdata = mcycle_reg[63:32];
			12'hF11: rdata = mvendorid;
			12'hF12: rdata = marchid;
			12'h341: rdata = mepc;
			12'h300: rdata = mstatus;
			12'h342: rdata = mcause;
			12'h305: rdata = mtvec;
			default: rdata = 32'h0;
		endcase
	end

endmodule
