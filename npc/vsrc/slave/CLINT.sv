`define CLINT_BASE  32'h0200_0000
`define CLINT_END   32'h0200_BFFF

`define MTIME_LOW   32'h0200_BFF8
`define MTIME_HIGH  32'h0200_BFFC

`define RESP_OKAY   2'b00
`define RESP_SLVERR 2'b10


module CLINT(
	axi4_lite_if axi_if
);

	import axi4_lite_pkg::*; // 引入 axi4_lite_pkg 包
  
  axi4_lite_Sreg axi_reg; // axi Slave 源寄存器

  // 寄存器连接至 axi_if
  assign axi_if.aw.AWREADY = axi_reg.aw.AWREADY;

  assign axi_if.w.WREADY = axi_reg.w.WREADY;

  assign axi_if.b.BVALID = axi_reg.b.BVALID;
  assign axi_if.b.BRESP = axi_reg.b.BRESP;

  assign axi_if.ar.ARREADY = axi_reg.ar.ARREADY;

  assign axi_if.r.RVALID = axi_reg.r.RVALID;
  assign axi_if.r.RDATA = axi_reg.r.RDATA;
  assign axi_if.r.RRESP = axi_reg.r.RRESP;

	// 读业务
  enum logic [1:0] {
    R_IDLE,
    R_WAIT_RESP,
    R_WAIT_R
  } r_state, r_next_state;

  always_ff @(posedge axi_if.ACLK) begin
    if (!axi_if.ARESETn) begin
      r_state <= R_IDLE;
    end else begin
      r_state <= r_next_state;
    end
  end
  // 辅助信号
  logic read_data_done; // 表示存储器读数据完成
  // assign read_data_done = 1'b1; // just for test

	// 事务推进使用的握手标志位
	wire ar_shake = axi_if.ar.ARVALID & axi_if.ar.ARREADY; 
	wire r_shake = axi_if.r.RVALID & axi_if.r.RREADY;

  // 状态转移逻辑
  always_comb begin 
    case(r_state)
      R_IDLE: begin
        if(ar_shake) begin
          r_next_state = R_WAIT_RESP;
        end else begin
          r_next_state = R_IDLE;
        end
      end
      R_WAIT_RESP: begin
        if(read_data_done) begin
          r_next_state = R_WAIT_R;
        end else begin
          r_next_state = R_WAIT_RESP;
        end
      end
      R_WAIT_R: begin
        if(r_shake) begin
          r_next_state = R_IDLE;
        end else begin
          r_next_state = R_WAIT_R;
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
        if(r_next_state == R_WAIT_RESP) begin
          axi_reg.ar.ARREADY <= 1'b0;
          raddr <= axi_if.ar.ARADDR;
          re <= 1'b1;
        end else begin // 简单复位逻辑
          axi_reg.ar.ARREADY <= 1'b1;
          re <= 1'b0;
          axi_reg.r.RVALID <= 1'b0;
        end
      end
      R_WAIT_RESP: begin
        if(r_next_state == R_WAIT_R) begin
          re <= 1'b0;
          axi_reg.r.RDATA <= rdata;
          axi_reg.r.RRESP <= rresp;
          axi_reg.r.RVALID <= 1'b1;
        end
      end
      R_WAIT_R: begin
        if(r_next_state == R_IDLE) begin
          axi_reg.r.RVALID <= 1'b0;
        end
      end
      
      default: begin
      end
    endcase
  end

	// 实际读取逻辑
	logic [31:0] rdata;
  logic [31:0] raddr;
	logic [1:0]  rresp;
  logic        re;

  always_ff @(posedge axi_if.ACLK) begin 
		if(~axi_if.ARESETn) begin
			rdata <= 32'h0;
			read_data_done <= 1'b0;
		end else
		if (r_state == R_WAIT_RESP) begin
			if(raddr == `MTIME_LOW) begin
				// $write("mtime_low: %x\n", mtime[31:0]);
				rdata <= mtime[31:0];
				read_data_done <= 1'b1;
				rresp <= `RESP_OKAY;
			end else
			if(raddr == `MTIME_HIGH) begin
				// $write("mtime_high: %x\n", mtime[63:32]);
				rdata <= mtime[63:32];
				read_data_done <= 1'b1;
				rresp <= `RESP_OKAY;
			end else begin
				rdata <= 32'h0;
				read_data_done <= 1'b1;
				rresp <= `RESP_SLVERR;
			end
		end else 
		begin
			rdata <= 32'h0;
			read_data_done <= 1'b0;
		end
  end

	// 寄存器定义
	reg [63:0] mtime;

	// 辅助寄存器
	reg [31:0] tick_counter;
	// 寄存器行为定义
	always_ff @(posedge axi_if.ACLK) begin
		if (!axi_if.ARESETn) begin
			mtime <= 64'h0;
			tick_counter <= 32'h0;
		end else begin
			if(tick_counter == 32'hFF) begin
				mtime <= mtime + 64'h1;
				tick_counter <= 32'h0;
			end else begin
				tick_counter <= tick_counter + 32'h1;
			end
		end
	end

endmodule