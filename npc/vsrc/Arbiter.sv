module Arbiter(
	axi4_lite_if axi_if_a, 
	axi4_lite_if axi_if_b,
	axi4_lite_if axi_if_out
);
	// 通道事务发起信号
	wire channel_a_req = axi_if_a.ar.ARVALID | axi_if_a.aw.AWVALID | axi_if_a.w.WVALID;
	wire channel_b_req = axi_if_b.ar.ARVALID | axi_if_b.aw.AWVALID | axi_if_b.w.WVALID;

	// 通道事务完成信号
	wire channel_a_done = (axi_if_a.r.RVALID & axi_if_a.r.RREADY) | (axi_if_a.b.BVALID & axi_if_a.b.BREADY);
	wire channel_b_done = (axi_if_b.r.RVALID & axi_if_b.r.RREADY) | (axi_if_b.b.BVALID & axi_if_b.b.BREADY);

	// FSM
	enum logic [1:0] {
		IDLE, 
		Forward_A,
		Forward_B
	} state, next_state;

	always_ff @(posedge axi_if_a.ACLK) begin // 认为两通道时钟同步
		if (!axi_if_a.ARESETn) begin
			state <= IDLE;
		end else begin
			state <= next_state;
		end
	end

	always_comb begin 
		case(state)
		IDLE: begin
			if (channel_a_req) begin
				next_state = Forward_A;
			end else if (channel_b_req) begin
				next_state = Forward_B;
			end else begin
				next_state = IDLE;
			end
		end

		Forward_A: begin
			if (channel_a_done) begin
				next_state = IDLE;
			end else begin
				next_state = Forward_A;
			end
		end

		Forward_B: begin
			if (channel_b_done) begin
				next_state = IDLE;
			end else begin
				next_state = Forward_B;
			end
		end

		default: begin
			next_state = IDLE;
		end
		endcase
	end

	always_comb begin
		case(state)
			IDLE: begin
				axi_if_a.slave_form_GND();
				axi_if_b.slave_form_GND();
				axi_if_out.master_form_GND();
			end

			Forward_A: begin
				axi_if_a.aw.AWREADY = axi_if_out.aw.AWREADY;
				axi_if_a.w.WREADY = axi_if_out.w.WREADY;
				axi_if_a.b.BVALID = axi_if_out.b.BVALID;
				axi_if_a.b.BRESP = axi_if_out.b.BRESP;
				axi_if_a.ar.ARREADY = axi_if_out.ar.ARREADY;
				axi_if_a.r.RVALID = axi_if_out.r.RVALID;
				axi_if_a.r.RDATA = axi_if_out.r.RDATA;
				axi_if_a.r.RRESP = axi_if_out.r.RRESP;

				axi_if_b.aw.AWREADY = 1'b0;
				axi_if_b.w.WREADY = 1'b0;
				axi_if_b.b.BVALID = 1'b0;
				axi_if_b.b.BRESP = 2'b00;
				axi_if_b.ar.ARREADY = 1'b0;
				axi_if_b.r.RVALID = 1'b0;
				axi_if_b.r.RDATA = 32'b0;
				axi_if_b.r.RRESP = 2'b00;

				axi_if_out.aw.AWVALID = axi_if_a.aw.AWVALID;
				axi_if_out.aw.AWADDR = axi_if_a.aw.AWADDR;
				axi_if_out.w.WVALID = axi_if_a.w.WVALID;
				axi_if_out.w.WDATA = axi_if_a.w.WDATA;
				axi_if_out.w.WSTRB = axi_if_a.w.WSTRB;
				axi_if_out.b.BREADY = axi_if_a.b.BREADY;
				axi_if_out.ar.ARVALID = axi_if_a.ar.ARVALID;
				axi_if_out.ar.ARADDR = axi_if_a.ar.ARADDR;
				axi_if_out.r.RREADY = axi_if_a.r.RREADY;
			end

			Forward_B: begin 
				axi_if_a.aw.AWREADY = 1'b0;
				axi_if_a.w.WREADY = 1'b0;
				axi_if_a.b.BVALID = 1'b0;
				axi_if_a.b.BRESP = 2'b00;
				axi_if_a.ar.ARREADY = 1'b0;
				axi_if_a.r.RVALID = 1'b0;
				axi_if_a.r.RDATA = 32'b0;
				axi_if_a.r.RRESP = 2'b00;

				axi_if_b.aw.AWREADY = axi_if_out.aw.AWREADY;
				axi_if_b.w.WREADY = axi_if_out.w.WREADY;
				axi_if_b.b.BVALID = axi_if_out.b.BVALID;
				axi_if_b.b.BRESP = axi_if_out.b.BRESP;
				axi_if_b.ar.ARREADY = axi_if_out.ar.ARREADY;
				axi_if_b.r.RVALID = axi_if_out.r.RVALID;
				axi_if_b.r.RDATA = axi_if_out.r.RDATA;
				axi_if_b.r.RRESP = axi_if_out.r.RRESP;

				axi_if_out.aw.AWVALID = axi_if_b.aw.AWVALID;
				axi_if_out.aw.AWADDR = axi_if_b.aw.AWADDR;
				axi_if_out.w.WVALID = axi_if_b.w.WVALID;
				axi_if_out.w.WDATA = axi_if_b.w.WDATA;
				axi_if_out.w.WSTRB = axi_if_b.w.WSTRB;
				axi_if_out.b.BREADY = axi_if_b.b.BREADY;
				axi_if_out.ar.ARVALID = axi_if_b.ar.ARVALID;
				axi_if_out.ar.ARADDR = axi_if_b.ar.ARADDR;
				axi_if_out.r.RREADY = axi_if_b.r.RREADY;
			end

			default: begin
				axi_if_a.slave_form_GND();
				axi_if_b.slave_form_GND();
				axi_if_out.master_form_GND();
			end
		endcase
	end
endmodule