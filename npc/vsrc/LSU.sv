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
	wire ram_we_control = ram_we_EX & en && (state == WAIT);
	
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
	// 1. 读写内存时，会进入 WAIT 状态
	// 2. 读写内存完成或者空闲时，会进入 IDLE 状态
	// 3. 在读写内存操作完成前一周期，会置 will_done 为 1
	enum logic [1:0]{
		IDLE,
		WAIT
	} state, next_state;

	always_ff @(posedge clk) begin
		state <= next_state;
	end

	always_comb begin 
		if(rst) begin
			next_state = IDLE;
		end else begin
			if(state == IDLE) begin
				if(is_ls_inst && en) begin // 需要读写内存
					next_state = WAIT;
				end else begin
					next_state = IDLE;
				end
			end else begin 
				if(respValid) begin // respValid 有效后返回 (等待响应)
					next_state = IDLE;
				end else begin
					next_state = WAIT;
				end
			end
		end
	end

	assign will_done = (next_state == IDLE); // 表示本周期结束后将访存结果会是有效的

	// simple_bus 其他控制信号
	wire reqValid = ram_re_EX || ram_we_EX;
	logic respValid;	

	// 对延迟的模拟
	// 附加延迟周期（delay_cycle) 0-255
	logic [7:0] delay_cycle, delay_counter;
	// assign delay_cycle = lfsr[7:0]; // 模拟一个随机数
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

endmodule