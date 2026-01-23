module RAM (
	axi4_lite_if axi_if,

	input  [31:0] ramReadData,
  output [31:0] ramReadAddr,
  output ramRe,

  output [31:0] ramWriteAddr,
  output [31:0] ramWriteData,
  output [3:0]  ramWriteMask,
  output ramWe
);
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
          axi_if.aw.AWREADY <= 1'b0;
          ramWriteAddr <= axi_if.aw.AWADDR;
        end else begin // 简单复位逻辑
          axi_if.aw.AWREADY <= 1'b1;
        end
      end

      S_DONE: begin
        if(aw_port_next_state == S_IDLE) begin
          axi_if.aw.AWREADY <= 1'b1;
        end
      end

      default: begin
      end
    endcase

    case(w_port_state)
      S_IDLE: begin
        if(w_port_next_state == S_DONE) begin
          axi_if.w.WREADY <= 1'b0;
          ramWriteMask <= axi_if.w.WSTRB;
          ramWriteData <= axi_if.w.WDATA;
        end else begin // 简单复位逻辑
          axi_if.w.WREADY <= 1'b1;
        end
      end

      S_DONE: begin
        if(w_port_next_state == S_IDLE) begin
          axi_if.w.WREADY <= 1'b1;
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
          ramWe <= 1'b1;
        end else begin  // 简单复位逻辑
          ramWe <= 1'b0;
          axi_if.b.BVALID <= 1'b0;
        end
      end
      W_WAIT_RESP: begin
        if(w_next_state == W_WAIT_B) begin
          ramWe <= 1'b0;
          axi_if.b.BRESP <= 2'b00; // OKAY
          axi_if.b.BVALID <= 1'b1;
        end
      end
      W_WAIT_B: begin
        if(w_next_state == W_IDLE) begin
          axi_if.b.BVALID <= 1'b0;
        end
      end
      default: begin
        ramWe <= 1'b0;
      end
    endcase
  end

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
  wire read_data_done; // 表示存储器读数据完成
  assign read_data_done = 1'b1; // just for test

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
          axi_if.ar.ARREADY <= 1'b0;
          ramReadAddr <= axi_if.ar.ARADDR;
          ramRe <= 1'b1;
        end else begin // 简单复位逻辑
          axi_if.ar.ARREADY <= 1'b1;
          ramRe <= 1'b0;
          axi_if.r.RVALID <= 1'b0;
        end
      end
      R_WAIT_RESP: begin
        if(r_next_state == R_WAIT_R) begin
          ramRe <= 1'b0;
          axi_if.r.RDATA <= ramReadData;
          axi_if.r.RRESP <= 2'b00; // OKAY
          axi_if.r.RVALID <= 1'b1;
        end
      end
      R_WAIT_R: begin
        if(r_next_state == R_IDLE) begin
          axi_if.r.RVALID <= 1'b0;
        end
      end
      
      default: begin
      end
    endcase
  end
endmodule