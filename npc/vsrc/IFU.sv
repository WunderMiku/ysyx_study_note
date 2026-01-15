module ysyx_25090244_IFU (
	input clk,
	input rst,

	input [31:0] pc,
	output reg [31:0] inst,

	// 总线连接
	simple_bus.master bus_out
);
	// FSM
	enum logic {
		IDLE,
		WAIT
	} state, next_state;

	always @(posedge clk or posedge rst) begin
		if (rst) begin
			state <= IDLE;
		end else begin
			state <= next_state;
		end	
	end

	always @(*) begin 
		if (state == IDLE) begin
			next_state = WAIT;
		end else begin
			next_state = IDLE;
		end
	end

	assign bus_out.valid = (state == WAIT) ? 1'b1 : 1'b0;

	import "DPI-C" function int pmem_read(input int raddr, input int len);
  import "DPI-C" function void pmem_write(input int waddr, input int wdata, input byte wmask);

  always @(*) begin
    if(!rst) begin
      inst = pmem_read(pc, 4);
    end else begin
      inst = 32'b0;
    end
  end

endmodule