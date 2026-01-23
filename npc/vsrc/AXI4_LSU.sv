interface axi4_aw_if ();
	logic AWVALID;
	logic AWREADY;
	logic [31:0] AWADDR;
//	logic [3:0] AWPROT;
endinterface

interface axi4_w_if ();
	logic WVALID;
	logic WREADY;
	logic [31:0] WDATA;
	logic [3:0] WSTRB;
endinterface

interface axi4_b_if ();
	logic BVALID;
	logic BREADY;
	logic [1:0] BRESP;
endinterface

interface axi4_ar_if ();
	logic ARVALID;
	logic ARREADY;
	logic [31:0] ARADDR;
//	logic [3:0] ARPROT;
endinterface

interface axi4_r_if ();
	logic RVALID;
	logic RREADY;
	logic [31:0] RDATA;
	logic [1:0] RRESP;
endinterface

interface axi4_lite_if ();
	logic ACLK;
	logic ARESETn;

	axi4_aw_if aw();
	axi4_w_if w();
	axi4_b_if b();
	axi4_ar_if ar();
	axi4_r_if r();

endinterface // axi4_lite_if


module axi4_lite_npcside (
	axi4_lite_if axi_if,

	input en, // top 控制
	output will_done, // npcside 告诉 top 是否将要完成

	input is_load,
	input is_store,
	input [31:0] raddr,
	input [31:0] waddr,
	input [31:0] wdata,
	input [3:0]  wstrb,

	output reg [1:0] wresp,
	output reg [1:0] rresp,
	output reg [31:0] rdata,

	output [1:0] lsu_wirte_state, lsu_read_state
);
	assign lsu_wirte_state = w_state;
	assign lsu_read_state = r_state;
	assign will_done = (is_load & r_will_done) | (is_store & w_will_done);

	// 写业务
	enum logic [1:0] { 
		W_IDLE,
		WAIT_AW,
		WAIT_W,
		WAIT_B
	} w_state, w_next_state;

	always_ff @(posedge axi_if.ACLK) begin
		if (!axi_if.ARESETn) begin
			w_state <= W_IDLE;
		end else begin
			w_state <= w_next_state;
		end
	end

	// 辅助信号
	wire w_will_done;

	wire have_w_req = en & is_store; // 用于判断是否需要写（W_IDLE -> WAIT_AW）

	// 事务推进使用的握手标志位
	wire aw_shake = axi_if.aw.AWVALID & axi_if.aw.AWREADY; 
	wire w_shake = axi_if.w.WVALID & axi_if.w.WREADY;
	wire b_shake = axi_if.b.BVALID & axi_if.b.BREADY;

	// 状态转移逻辑
	always_comb begin 
		case(w_state)
		W_IDLE: begin
			if (have_w_req) begin
				w_next_state = WAIT_AW;
			end else begin
				w_next_state = W_IDLE;
			end
		end

		WAIT_AW: begin
			if (aw_shake) begin
				w_next_state = WAIT_W;
			end else begin
				w_next_state = WAIT_AW;
			end
		end

		WAIT_W: begin
			if (w_shake) begin
				w_next_state = WAIT_B;
			end else begin
				w_next_state = WAIT_W;
			end
		end

		WAIT_B: begin
			if (b_shake) begin
				w_next_state = W_IDLE;
			end else begin
				w_next_state = WAIT_B;
			end
		end

		default: begin
			w_next_state = W_IDLE;
		end
		endcase
	end

	// 状态机对端口寄存器的控制
	always_ff @(posedge axi_if.ACLK) begin
		case(w_state) 
			W_IDLE: begin
				if(w_next_state == WAIT_AW) begin
					axi_if.aw.AWVALID <= 1'b1;
					axi_if.aw.AWADDR <= waddr;
				end else begin // 简单复位逻辑
					axi_if.aw.AWVALID <= 1'b0;
					axi_if.w.WVALID <= 1'b0;
					axi_if.b.BREADY <= 1'b0;
				end
			end

			WAIT_AW: begin
				if(w_next_state == WAIT_W) begin
					axi_if.aw.AWVALID <= 1'b0;
					axi_if.w.WVALID <= 1'b1;
					axi_if.w.WDATA <= wdata;
					axi_if.w.WSTRB <= wstrb;
				end
			end

			WAIT_W: begin
				if(w_next_state == WAIT_B) begin
					axi_if.w.WVALID <= 1'b0;
					axi_if.b.BREADY <= 1'b1;
				end
			end

			WAIT_B: begin
				if(w_next_state == W_IDLE) begin
					axi_if.b.BREADY <= 1'b0;
					wresp <= axi_if.b.BRESP;
				end
			end

			default: begin
				axi_if.aw.AWVALID <= 1'b0;
				axi_if.w.WVALID <= 1'b0;
				axi_if.b.BREADY <= 1'b0;
			end
		endcase
	end

	// 告诉 top 是否将要完成
	assign w_will_done = (w_state == WAIT_B) & (w_next_state == W_IDLE); 

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

	wire have_r_req = en & is_load; // 用于判断是否需要写（R_IDLE -> WAIT_AR）

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