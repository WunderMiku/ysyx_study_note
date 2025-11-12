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
	input we1_in,
	input [11:0] waddr1,
	input [31:0] wdata1,

	// 调试接口
	output reg [31:0] out_csr [3:0]
);
	// 避免写冲突
	wire we1 = (waddr1 == waddr) ? 0 : we1_in;
	
	// mcycle 
	reg [63:0] mcycle_reg;
	// mvendorid
	reg [31:0] mvendorid = 32'h79737978;
	// marchid
	reg [31:0] marchid = 32'h17ED8C4;
	// mepc
	reg [31:0] mepc;
	// mstatus
	reg [31:0] mstatus;
	// mcause
	reg [31:0] mcause;
	// mtvec
	reg [31:0] mtvec;


// mcycle write control
	always @(posedge clk or posedge rst) begin
		if (rst) begin // reset
			mcycle_reg <= 64'h0;
		end else 

		if (we1 | we) begin  // write operation
			// port 0
			if(we & (waddr == 12'hB00)) begin // set lower 32 bits
				mcycle_reg[31:0] <= wdata;
			end else
			if(we & (waddr == 12'hB80)) begin // set upper 32 bits
				mcycle_reg[63:32] <= wdata;
			end 

			// port 1
			if(we1 & (waddr1 == 12'hB00)) begin // set lower 32 bits(1)
				mcycle_reg[31:0] <= wdata1;
			end else
			if(we1 & (waddr1 == 12'hB80)) begin // set upper 32 bits(1)
				mcycle_reg[63:32] <= wdata1;
			end 
		end else

		begin // normal increment
			mcycle_reg <= mcycle_reg + 1;   // increment every cycle
		end
	end

// mstatus write control
	always @(posedge clk or posedge rst) begin
		if (rst) begin
			mstatus <= 32'h1800;
		end else begin
			if(we & (waddr == 12'h300)) begin
				mstatus <= wdata;
			end 

			if(we1 & (waddr1 == 12'h300)) begin
				mstatus <= wdata1;
			end
		end
	end

// other csrs write control
	always @(posedge clk) begin 
		if(we) begin
			case(waddr)
				12'h341: mepc <= wdata;
				12'h342: mcause <= wdata;
				12'h305: mtvec <= wdata;
				default: ;
			endcase
		end

		if(we1) begin
			case(waddr1)
				12'h341: mepc <= wdata1;
				12'h342: mcause <= wdata1;
				12'h305: mtvec <= wdata1;
				default: ;
			endcase
		end
	end

// all csrs read operation
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

// debug output
	assign out_csr[0] = mepc[31:0];
	assign out_csr[1] = mstatus[31:0];
	assign out_csr[2] = mcause[31:0];
	assign out_csr[3] = mtvec[31:0];

endmodule
