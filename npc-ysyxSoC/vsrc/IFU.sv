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
	// 1. 需要读写内存时，会进入 WAIT 状态，-+->其中若存储器可以接受请求 -+-> 还没有给出有效结果，则进入WAIT_GET_RESP 状态,
	//                                  |                        +-> 给出有效响应，则直接回到IDLE 状态。
	//                                  +--> 若存储器暂时无法接受请求，则进入WAIT_REQ_SEND 状态。
	// 2. 读写内存完成或者空闲时，会进入 IDLE 状态
	// 3. 在读写内存操作完成前一周期，会置 will_done 为 1
	enum logic [1:0]{
		IDLE,
		WAIT_REQ_SEND,
		WAIT_GET_RESP
	} state, next_state;

	always_ff @(posedge clk) begin
		state <= next_state;
	end

	always_comb begin 
		if(rst) begin
			next_state = IDLE;
		end else begin
			case (state)
				IDLE: begin
					if (en) begin // 有访存请求
						if(respValid) begin
							next_state = IDLE;  // 理想的无延迟访存
						end else begin
							if(reqReady) begin
								next_state = WAIT_GET_RESP; // 存储器可以接受请求
							end else begin
								next_state = WAIT_REQ_SEND; // 存储器不能接受请求
							end
						end
					end else begin  // 无访存请求
						next_state = IDLE;
					end
				end

				WAIT_REQ_SEND: begin
					if (reqReady) begin
						next_state = WAIT_GET_RESP;  // 存储器可以接受请求
					end else begin
						next_state = WAIT_REQ_SEND;  // 存储器依然不能接受请求
					end
				end

				WAIT_GET_RESP: begin
						if (respValid) begin
							next_state = IDLE;  // 访存完成
						end else begin
							next_state = WAIT_GET_RESP;  // 等待存储器返回有效数据
						end
					end

				default: begin
					next_state = IDLE;
				end
			endcase
		end
	end

	assign will_done = en & (next_state == IDLE); // 表示本周期结束后将访存完成

	// simple_bus 其他控制信号
	wire reqValid = 1'b1;
	logic respValid;	

	// reqReady = 1 表示：存储器准备好接受请求
	// respReady = 1 表示：处理器准备好接受返回响应
	wire reqReady;
	wire respReady = 1'b1; // 暂定为始终有效

	logic [7:0] delay_cycle_req, delay_counter_req;
	// assign delay_cycle_req = lfsr[15:8]; // 模拟一个随机数
	assign delay_cycle_req = 8'd1;  // 模拟存储器准备接受请求带来的延迟（设置为0，表示无延迟）
	assign reqReady = (delay_counter_req == 8'b0);
	
	// reqReady 模拟
	always_ff @(posedge clk or posedge rst) begin
		if (rst) begin
			delay_counter_req <= delay_cycle_req;
		end else begin
			if (next_state == WAIT_REQ_SEND) begin
				if (delay_counter_req >= 8'b1) begin
					delay_counter_req <= delay_counter_req - 1'b1;
				end else begin
					delay_counter_req <= delay_counter_req;
				end
			end else begin
				delay_counter_req <= delay_cycle_req;
			end
		end
	end

	// 对延迟的模拟
	// 附加延迟周期（delay_cycle_resp) 0-255
	logic [7:0] delay_cycle_resp, delay_counter_resp;
	// assign delay_cycle_resp = lfsr[7:0]; // 模拟一个随机数
	assign delay_cycle_resp = 8'd1; // 模拟存储器返回有效数据所需的延迟
	assign respValid = (delay_counter_resp == 8'b0);

	// respValid 模拟
	always_ff @(posedge clk or posedge rst) begin
		if (rst) begin
			delay_counter_resp <= delay_cycle_resp;
		end else begin
			if (next_state == WAIT_GET_RESP) begin
				if (delay_counter_resp >= 8'b1) begin
					delay_counter_resp <= delay_counter_resp - 1'b1;
				end else begin
					delay_counter_resp <= delay_counter_resp;
				end
			end else begin
				delay_counter_resp <= delay_cycle_resp;
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