module ysyx_25090244_IFU (
	input clk,
	input rst,
	
	// Top 控制
	input en,
	output will_done,

	simple_bus_IFU.IFU_port bus_in,

	// 调试接口
	output [1:0] out_state
);
  // 调试接口
	assign out_state = state;

	// ======== FSM 定义 ========
	enum logic [1:0] {
		IDLE,
		WAIT
	} state;

	always_ff @(posedge clk or posedge rst) begin
		if(rst) begin
			state <= IDLE;
		end else begin
			case(state)
				IDLE: begin
					if(en) begin
						state <= WAIT;  // 可以取指
					end else begin
						state <= IDLE;
					end
				end
				WAIT: begin
					if(respValid) begin
						state <= IDLE;  // 完成取指
					end else begin
						state <= WAIT;
					end
				end
				default: begin
					state <= IDLE;
				end
			endcase
		end
	end 
	assign will_done = (state == WAIT) & respValid;

	// simple_bus 其他控制信号
	wire reqValid = 1'b1;
	logic respValid;	

	// 对延迟的模拟
	// 附加延迟周期（delayCycle) 0-255
	logic [7:0] delay_cycle, delay_counter;
	// assign delay_cycle = lfsr[7:0];
	assign delay_cycle = 8'h0;
	assign respValid = (delay_counter == 8'b0);

	always_ff @(posedge clk or posedge rst) begin
		if (rst) begin
			delay_counter <= delay_cycle;
		end else begin
			if (state == WAIT) begin
				if (delay_counter >= 8'b1) begin
					delay_counter <= delay_counter - 1'b1;
				end else begin
					delay_counter <= delay_counter;
				end
			end else begin
				delay_counter <= delay_cycle;
			end
		end
	end

	// 随机延时的生成（LSFR）
	logic [15:0] lfsr;
  logic lsfr_en, feedback;
	assign lsfr_en = 1;
  assign feedback = lfsr[15] ^ lfsr[13] ^ lfsr[12] ^ lfsr[10];

  always_ff @(posedge clk or posedge rst) begin
    if (rst) begin
      lfsr <= 16'h12;      // 不能为 0
    end else if (lsfr_en) begin
      lfsr <= {lfsr[14:0], feedback};
    end
  end

	// ======== IFU 读取逻辑 ========

	import "DPI-C" function int pmem_read(input int raddr, input int len);
  import "DPI-C" function void pmem_write(input int waddr, input int wdata, input byte wmask);

  always @(*) begin
    if(!rst) begin
      bus_in.ifu_rdata = pmem_read(bus_in.ifu_raddr, 4);
    end else begin
      bus_in.ifu_rdata = 32'b0;
    end
  end

endmodule