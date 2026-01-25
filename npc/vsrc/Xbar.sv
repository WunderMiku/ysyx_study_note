module Xbar (
	axi4_lite_if axi_if_in,

	axi4_lite_if axi_if_ram,
	axi4_lite_if axi_if_uart,
	axi4_lite_if axi_if_clint
);
	// Xbar, 这里因为LSU的钦定固定写操作流程，这里可以设计比较简单（也就意味着并不是很符合AXI4-Lite标准，但如果一方满足AXI4-Lite标准，那么Xbar也可以正常运行）
	// 通道事务发起信号
	wire in_wreq = axi_if_in.aw.AWVALID;
	wire in_rreq = axi_if_in.ar.ARVALID;

	// 通道事务完成信号
	wire in_done = (axi_if_in.r.RVALID & axi_if_in.r.RREADY) | (axi_if_in.b.BVALID & axi_if_in.b.BREADY);

	enum logic [2:0] {
		IDLE,
		Forward_Ram,
		Forward_Uart,
		Forward_Clint,
		Decerr
	} state, next_state;

	always_ff @(posedge axi_if_in.ACLK) begin
		if (~axi_if_in.ARESETn) begin
			state <= IDLE;
		end else begin
			state <= next_state;
		end
	end

	// 便利信号
	wire [31:0] req_addr = in_wreq ? axi_if_in.aw.AWADDR : axi_if_in.ar.ARADDR; // 仅在有请求时使用

	always_comb begin 
		case(state) 
			IDLE: begin
				if (in_wreq | in_rreq) begin
					if (in_wreq || in_rreq) begin
						if (req_addr >= 32'h8000_0000 && req_addr <= 32'h8800_0000) begin
								next_state = Forward_Ram;
						end else if (req_addr == 32'h1000_0000) begin
								next_state = Forward_Uart;
						end else if (req_addr >= 32'h0200_0000 && req_addr <= 32'h0200_BFFF) begin
								next_state = Forward_Clint;
						end else begin
								next_state = Decerr;
						end
					end
				end else begin
					next_state = IDLE;
				end
			end

			Forward_Ram: begin
				if (in_done) begin
					next_state = IDLE;
				end else begin
					next_state = Forward_Ram;
				end
			end

			Forward_Uart: begin
				if (in_done) begin
					next_state = IDLE;
				end else begin
					next_state = Forward_Uart;
				end
			end

			Decerr: begin
				if (in_done) begin
					next_state = IDLE;
				end else begin
					next_state = Decerr;
				end
			end

			Forward_Clint: begin
				if (in_done) begin
					next_state = IDLE;
				end else begin
					next_state = Forward_Clint;
				end
			end

			default: begin
				next_state = IDLE;
			end
		endcase
	end

	always_comb begin
		// 默认信号
		axi_if_in.slave_form_GND();
		axi_if_uart.master_form_GND();
		axi_if_ram.master_form_GND();
		axi_if_clint.master_form_GND();

		case(state)
			IDLE: begin
			end

			Forward_Ram: begin 
				axi_if_in.aw.AWREADY = axi_if_ram.aw.AWREADY;
				axi_if_in.w.WREADY = axi_if_ram.w.WREADY;
				axi_if_in.b.BVALID = axi_if_ram.b.BVALID;
				axi_if_in.b.BRESP = axi_if_ram.b.BRESP;
				axi_if_in.ar.ARREADY = axi_if_ram.ar.ARREADY;
				axi_if_in.r.RVALID = axi_if_ram.r.RVALID;
				axi_if_in.r.RDATA = axi_if_ram.r.RDATA;
				axi_if_in.r.RRESP = axi_if_ram.r.RRESP;

				axi_if_ram.aw.AWVALID = axi_if_in.aw.AWVALID;
				axi_if_ram.aw.AWADDR = axi_if_in.aw.AWADDR;
				axi_if_ram.w.WVALID = axi_if_in.w.WVALID;
				axi_if_ram.w.WDATA = axi_if_in.w.WDATA;
				axi_if_ram.w.WSTRB = axi_if_in.w.WSTRB;
				axi_if_ram.b.BREADY = axi_if_in.b.BREADY;
				axi_if_ram.ar.ARVALID = axi_if_in.ar.ARVALID;
				axi_if_ram.ar.ARADDR = axi_if_in.ar.ARADDR;
				axi_if_ram.r.RREADY = axi_if_in.r.RREADY;
			end

			Forward_Uart: begin
				axi_if_in.aw.AWREADY = axi_if_uart.aw.AWREADY;
				axi_if_in.w.WREADY = axi_if_uart.w.WREADY;
				axi_if_in.b.BVALID = axi_if_uart.b.BVALID;
				axi_if_in.b.BRESP = axi_if_uart.b.BRESP;
				axi_if_in.ar.ARREADY = axi_if_uart.ar.ARREADY;
				axi_if_in.r.RVALID = axi_if_uart.r.RVALID;
				axi_if_in.r.RDATA = axi_if_uart.r.RDATA;
				axi_if_in.r.RRESP = axi_if_uart.r.RRESP;

				axi_if_uart.aw.AWVALID = axi_if_in.aw.AWVALID;
				axi_if_uart.aw.AWADDR = axi_if_in.aw.AWADDR;
				axi_if_uart.w.WVALID = axi_if_in.w.WVALID;
				axi_if_uart.w.WDATA = axi_if_in.w.WDATA;
				axi_if_uart.w.WSTRB = axi_if_in.w.WSTRB;
				axi_if_uart.b.BREADY = axi_if_in.b.BREADY;
				axi_if_uart.ar.ARVALID = axi_if_in.ar.ARVALID;
				axi_if_uart.ar.ARADDR = axi_if_in.ar.ARADDR;
				axi_if_uart.r.RREADY = axi_if_in.r.RREADY;
			end

			Forward_Clint: begin 
				axi_if_in.aw.AWREADY = axi_if_clint.aw.AWREADY;
				axi_if_in.w.WREADY = axi_if_clint.w.WREADY;
				axi_if_in.b.BVALID = axi_if_clint.b.BVALID;
				axi_if_in.b.BRESP = axi_if_clint.b.BRESP;
				axi_if_in.ar.ARREADY = axi_if_clint.ar.ARREADY;
				axi_if_in.r.RVALID = axi_if_clint.r.RVALID;
				axi_if_in.r.RDATA = axi_if_clint.r.RDATA;
				axi_if_in.r.RRESP = axi_if_clint.r.RRESP;

				axi_if_clint.aw.AWVALID = axi_if_in.aw.AWVALID;
				axi_if_clint.aw.AWADDR = axi_if_in.aw.AWADDR;
				axi_if_clint.w.WVALID = axi_if_in.w.WVALID;
				axi_if_clint.w.WDATA = axi_if_in.w.WDATA;
				axi_if_clint.w.WSTRB = axi_if_in.w.WSTRB;
				axi_if_clint.b.BREADY = axi_if_in.b.BREADY;
				axi_if_clint.ar.ARVALID = axi_if_in.ar.ARVALID;
				axi_if_clint.ar.ARADDR = axi_if_in.ar.ARADDR;
				axi_if_clint.r.RREADY = axi_if_in.r.RREADY;
			end

			Decerr: begin
				axi_if_in.aw.AWREADY = 1'b1;
				axi_if_in.w.WREADY = 1'b1;
				axi_if_in.b.BVALID = 1'b1;
				axi_if_in.b.BRESP = 2'b11;

				axi_if_in.ar.ARREADY = 1'b1;
				axi_if_in.r.RVALID = 1'b1;
				axi_if_in.r.RRESP = 2'b11;
			end

			default: begin
			end
		endcase
	end
 
endmodule