module axi4_lite_ifu (
	axi4_lite_if axi_if,

	input en, // top 控制
	output will_done, // npcside 告诉 top 是否将要完成

	input [31:0] raddr,

	output reg [1:0] rresp,
	output reg [31:0] rdata,

	output [1:0] ifu_state
);
	import axi4_lite_pkg::*; // 引入 axi4_lite_pkg 包
	axi4_lite_Mreg axi_reg; // axi Master 源寄存器

	// 寄存器连接至 axi_if
	assign axi_if.aw.AWVALID = axi_reg.aw.AWVALID;
	assign axi_if.aw.AWADDR = axi_reg.aw.AWADDR;

	assign axi_if.w.WVALID = axi_reg.w.WVALID;
	assign axi_if.w.WDATA = axi_reg.w.WDATA;
	assign axi_if.w.WSTRB = axi_reg.w.WSTRB;

	assign axi_if.b.BREADY = axi_reg.b.BREADY;

	assign axi_if.ar.ARVALID = axi_reg.ar.ARVALID;
	assign axi_if.ar.ARADDR = axi_reg.ar.ARADDR;

	assign axi_if.r.RREADY = axi_reg.r.RREADY;


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
					axi_reg.ar.ARVALID <= 1'b1;
					axi_reg.ar.ARADDR <= raddr;
				end else begin // 简单复位逻辑
					axi_reg.ar.ARVALID <= 1'b0;
					axi_reg.r.RREADY <= 1'b0;
				end
			end

			WAIT_AR: begin
				if(r_next_state == WAIT_R) begin
					axi_reg.ar.ARVALID <= 1'b0;
					axi_reg.r.RREADY <= 1'b1;
				end
			end

			WAIT_R: begin
				if(r_next_state == R_IDLE) begin
					axi_reg.r.RREADY <= 1'b0;
					rdata <= axi_if.r.RDATA;
					rresp <= axi_if.r.RRESP;
				end
			end

			default: begin
				axi_reg.ar.ARVALID <= 1'b0;
				axi_reg.r.RREADY <= 1'b0;
			end
		endcase
	end

	// 告诉 top 是否将要完成
	assign r_will_done = (r_state == WAIT_R) & (r_next_state == R_IDLE);
endmodule