module UART(
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

  // 写业务
  enum logic [1:0] {
    S_IDLE,
    S_DONE
  } aw_port_state, aw_port_next_state, w_port_state, w_port_next_state;

  enum logic [1:0] {
    W_IDLE,
    W_WAIT_RESP,
    W_WAIT_B
  } w_state, w_next_state;

   always_ff @(posedge axi_if.ACLK) begin
    if (!axi_if.ARESETn) begin
      aw_port_state <= S_IDLE; w_port_state <= S_IDLE;
      w_state <= W_IDLE;
    end else begin
      aw_port_state <= aw_port_next_state; w_port_state <= w_port_next_state;
      w_state <= w_next_state;
    end
  end
  // 辅助信号
  wire write_data_done; // 表示存储器是否完成写操作
  assign write_data_done = 1'b1; // just for test

  wire w_will_done = (w_state == W_WAIT_B) & (w_next_state == W_IDLE); 
  // 事务推进使用的握手标志位
	wire aw_shake = axi_if.aw.AWVALID & axi_if.aw.AWREADY; 
	wire w_shake = axi_if.w.WVALID & axi_if.w.WREADY;
	wire b_shake = axi_if.b.BVALID & axi_if.b.BREADY;

  // 状态转移逻辑 (port)
  always_comb begin 
    case(aw_port_state)
    S_IDLE: begin
      if (aw_shake) begin
        aw_port_next_state = S_DONE;
      end else begin
        aw_port_next_state = S_IDLE;
      end
    end

    S_DONE: begin
      if (w_will_done) begin
        aw_port_next_state = S_IDLE;
      end else begin
        aw_port_next_state = S_DONE;
      end
    end

    default: begin
      aw_port_next_state = S_IDLE;
    end
    endcase

    case(w_port_state)
    S_IDLE: begin
      if (w_shake) begin
        w_port_next_state = S_DONE;
      end else begin
        w_port_next_state = S_IDLE;
      end
    end

    S_DONE: begin
      if (w_will_done) begin
        w_port_next_state = S_IDLE;
      end else begin
        w_port_next_state = S_DONE;
      end
    end

    default: begin
      w_port_next_state = S_IDLE;
    end
    endcase
  end
  // 状态转移逻辑 (main)
  always_comb begin 
    case(w_state)
    W_IDLE: begin
      if ((aw_port_state == S_DONE) & (w_port_state == S_DONE)) begin
        w_next_state = W_WAIT_RESP;
      end else begin
        w_next_state = W_IDLE;
      end
    end

    W_WAIT_RESP: begin
      if (write_data_done) begin
        w_next_state = W_WAIT_B;
      end else begin
        w_next_state = W_WAIT_RESP;
      end
    end

    W_WAIT_B: begin
      if (b_shake) begin
        w_next_state = W_IDLE;
      end else begin
        w_next_state = W_WAIT_B;
      end
    end

    default: begin
      w_next_state = W_IDLE;
    end

    endcase
  end

  // 状态机对端口寄存器的控制 (port)
  always_ff @(posedge axi_if.ACLK) begin
    case(aw_port_state)
      S_IDLE: begin
        if(aw_port_next_state == S_DONE) begin
          axi_reg.aw.AWREADY <= 1'b0;
          waddr <= axi_if.aw.AWADDR;
        end else begin // 简单复位逻辑
          axi_reg.aw.AWREADY <= 1'b1;
        end
      end

      S_DONE: begin
        if(aw_port_next_state == S_IDLE) begin
          axi_reg.aw.AWREADY <= 1'b1;
        end
      end

      default: begin
      end
    endcase

    case(w_port_state)
      S_IDLE: begin
        if(w_port_next_state == S_DONE) begin
          axi_reg.w.WREADY <= 1'b0;
          wmask <= axi_if.w.WSTRB;
          wdata <= axi_if.w.WDATA;
        end else begin // 简单复位逻辑
          axi_reg.w.WREADY <= 1'b1;
        end
      end

      S_DONE: begin
        if(w_port_next_state == S_IDLE) begin
          axi_reg.w.WREADY <= 1'b1;
        end
      end

      default: begin
      end
    endcase
  end

  // 状态机对端口寄存器的控制 (main)
  always_ff @(posedge axi_if.ACLK) begin
    case(w_state)
      W_IDLE: begin
        if(w_next_state == W_WAIT_RESP) begin
          we <= 1'b1;
        end else begin  // 简单复位逻辑
          we <= 1'b0;
          axi_reg.b.BVALID <= 1'b0;
        end
      end
      W_WAIT_RESP: begin
        if(w_next_state == W_WAIT_B) begin
          we <= 1'b0;
          axi_reg.b.BRESP <= 2'b00; // OKAY
          axi_reg.b.BVALID <= 1'b1;
        end
      end
      W_WAIT_B: begin
        if(w_next_state == W_IDLE) begin
          axi_reg.b.BVALID <= 1'b0;
        end
      end
      default: begin
        we <= 1'b0;
      end
    endcase
  end

	// Uart 功能实现
  logic [31:0] waddr;
  logic [31:0] wdata;
  logic [3:0]  wmask;
  logic we;

	wire [31:0] wdata_after_mask = {{{8{wmask[3]}} & wdata[31:24]}, {{8{wmask[2]}} & wdata[23:16]}, {{8{wmask[1]}} & wdata[15:8]}, {{8{wmask[0]}} & wdata[7:0]}};
	always_ff @(posedge axi_if.ACLK) begin
		if (we) begin
			$write("%c", wdata_after_mask[7:0]);
		end
	end

endmodule