module ysyx_25090244_LSU (
	input clk,
	input rst,

	// 写端口控制信号
	input  ram_we_EX,
	input  [31:0] ram_write_data_EX,
	input  [31:0] ram_write_addr_EX,
	input  [3:0]  ram_write_mask_EX,

	// 读端口控制信号
	input  ram_re_EX,
	input  [31:0] ram_read_addr_EX,

	// LSU 输出
	output [31:0] ram_read_data,

	// 临时辅助信号（因为现在MEM读写端口实际是Top的端口实现的）
	// 写端口
	output 			    ram_we_Top,
	output [31:0] 	ram_write_data_Top,
	output [31:0] 	ram_write_addr_Top,
	output [3:0]  	ram_write_mask_Top,

	// 读端口
	output 			    ram_re_Top,
	output [31:0] 	ram_read_addr_Top,
	input  [31:0] 	ram_read_data_Top,

	// Top 控制信号
	input en,
	output will_done,

	// 调试接口
	output [1:0] out_state
);
  // 调试接口
	assign out_state = state;

	// 控制信号
	wire ram_we_control = ram_we_EX & en && will_done;
	
	// 辅助控制信号
	wire is_ls_inst = ram_re_EX || ram_we_EX;


	// 写端口连接
	assign ram_we_Top = ram_we_control;
	assign ram_write_data_Top = ram_write_data_EX;
	assign ram_write_addr_Top = ram_write_addr_EX;
	assign ram_write_mask_Top = ram_write_mask_EX;

	// 读端口连接
	assign ram_re_Top = ram_re_EX;
	assign ram_read_addr_Top = ram_read_addr_EX;

	// 读数据输出
	assign ram_read_data = ram_read_data_Top;

	// FSM：
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
					if (is_ls_inst & en) begin // 有访存请求
						if(respReady & respValid) begin
							next_state = IDLE;  // 理想的无延迟访存
						end else begin
							if(reqReady & reqValid) begin
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
					if (reqReady & reqValid) begin
						next_state = WAIT_GET_RESP;  // 存储器可以接受请求
					end else begin
						next_state = WAIT_REQ_SEND;  // 存储器依然不能接受请求
					end
				end

				WAIT_GET_RESP: begin
						if (respReady & respValid) begin
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
	wire reqValid = ram_re_EX || ram_we_EX;
	logic respValid;	

	// reqReady = 1 表示：存储器准备好接受请求
	// respReady = 1 表示：处理器准备好接受返回响应
	wire reqReady;
	wire respReady = 1'b1; // 暂定为始终有效

	logic [7:0] delay_cycle_req, delay_counter_req;
	// assign delay_cycle_req = lfsr[15:8]; // 模拟一个随机数
	assign delay_cycle_req = 8'd10;  // 模拟存储器准备接受请求带来的延迟（设置为0，表示无延迟）
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

endmodule