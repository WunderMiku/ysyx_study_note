module axi4_lite_ifu (
	axi4_lite_if axi_if,

	input en, // top 控制
	output will_done, // npcside 告诉 top 是否将要完成

	input [31:0] raddr,

	output reg [1:0] rresp,
	output reg [31:0] rdata,

	output [1:0] ifu_state
);
	assign ifu_state = r_state;
	assign will_done = en & r_will_done;

	// 读业务
	enum logic [1:0] { 
		R_IDLE,
		WAIT_AR,
		WAIT_R
	} r_state, r_next_state;

	always_ff @(posedge axi_if.ACLK) begin
		if (!axi_if.ARESETn) begin
			r_state <= R_IDLE;
		end else begin
			r_state <= r_next_state;
		end
	end

	// 辅助信号
	wire r_will_done;

	wire have_r_req = en; // 用于判断是否需要写（R_IDLE -> WAIT_AR）

	// 事务推进使用的握手标志位
	wire ar_shake = axi_if.ar.ARVALID & axi_if.ar.ARREADY; 
	wire r_shake = axi_if.r.RVALID & axi_if.r.RREADY;

	// 状态转移逻辑
	always_comb begin 
		case(r_state)
		R_IDLE: begin
			if (have_r_req) begin
				r_next_state = WAIT_AR;
			end else begin
				r_next_state = R_IDLE;
			end
		end

		WAIT_AR: begin
			if (ar_shake) begin
				r_next_state = WAIT_R;
			end else begin
				r_next_state = WAIT_AR;
			end
		end

		WAIT_R: begin
			if (r_shake) begin
				r_next_state = R_IDLE;
			end else begin
				r_next_state = WAIT_R;
			end
		end

		default: begin
			r_next_state = R_IDLE;
		end
		endcase
	end

	// 状态机对端口寄存器的控制
	always_ff @(posedge axi_if.ACLK) begin
		case(r_state) 
			R_IDLE: begin
				if(r_next_state == WAIT_AR) begin
					axi_if.ar.ARVALID <= 1'b1;
					axi_if.ar.ARADDR <= raddr;
				end else begin // 简单复位逻辑
					axi_if.ar.ARVALID <= 1'b0;
					axi_if.r.RREADY <= 1'b0;
				end
			end

			WAIT_AR: begin
				if(r_next_state == WAIT_R) begin
					axi_if.ar.ARVALID <= 1'b0;
					axi_if.r.RREADY <= 1'b1;
				end
			end

			WAIT_R: begin
				if(r_next_state == R_IDLE) begin
					axi_if.r.RREADY <= 1'b0;
					rdata <= axi_if.r.RDATA;
					rresp <= axi_if.r.RRESP;
				end
			end


			default: begin
				axi_if.ar.ARVALID <= 1'b0;
				axi_if.r.RREADY <= 1'b0;
			end
		endcase
	end

	// 告诉 top 是否将要完成
	assign r_will_done = (r_state == WAIT_R) & (r_next_state == R_IDLE);
endmodule