module ROM (
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
          ramReadAddr <= axi_if.ar.ARADDR;
          ramRe <= 1'b1;
        end else begin // 简单复位逻辑
          axi_reg.ar.ARREADY <= 1'b1;
          ramRe <= 1'b0;
          axi_reg.r.RVALID <= 1'b0;
        end
      end
      R_WAIT_RESP: begin
        if(r_next_state == R_WAIT_R) begin
          ramRe <= 1'b0;
          axi_reg.r.RDATA <= ramReadData;
          axi_reg.r.RRESP <= 2'b00; // OKAY
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
	logic [31:0] ramReadData;
  logic [31:0] ramReadAddr;
  logic ramRe; // 不考虑读使能

	import "DPI-C" function int pmem_read(input int raddr, input int len);
  import "DPI-C" function void pmem_write(input int waddr, input int wdata, input byte wmask);

  always_ff @(posedge axi_if.ACLK) begin 
		if(~axi_if.ARESETn) begin
			ramReadData <= 32'h0;
			read_data_done <= 1'b0;
		end else
		if (r_state == R_WAIT_RESP) begin
			ramReadData <= pmem_read(ramReadAddr, 4);
			read_data_done <= 1'b1;
		end else 
		begin
			ramReadData <= 32'h0;
			read_data_done <= 1'b0;
		end
  end
endmodule