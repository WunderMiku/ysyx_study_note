module ysyx_25090244 (
  // 时钟与复位
  input        clock,
  input        reset,           // 高电平有效
  input        io_interrupt,    // 外部中断

  // AXI4 Master接口
  input         io_master_awready,
  output        io_master_awvalid,
  output [31:0] io_master_awaddr,
  output [3:0]  io_master_awid,
  output [7:0]  io_master_awlen,
  output [2:0]  io_master_awsize,
  output [1:0]  io_master_awburst,

  input         io_master_wready,
  output        io_master_wvalid,
  output [31:0] io_master_wdata,
  output [3:0]  io_master_wstrb,
  output        io_master_wlast,

  output        io_master_bready,
  input         io_master_bvalid,
  input  [1:0]  io_master_bresp,
  input  [3:0]  io_master_bid,

  input         io_master_arready,
  output        io_master_arvalid,
  output [31:0] io_master_araddr,
  output [3:0]  io_master_arid,
  output [7:0]  io_master_arlen,
  output [2:0]  io_master_arsize,
  output [1:0]  io_master_arburst,

  output        io_master_rready,
  input         io_master_rvalid,
  input  [1:0]  io_master_rresp,
  input  [31:0] io_master_rdata,
  input         io_master_rlast,
  input  [3:0]  io_master_rid,

  // AXI4 Slave接口
  output        io_slave_awready,
  input         io_slave_awvalid,
  input  [31:0] io_slave_awaddr,
  input  [3:0]  io_slave_awid,
  input  [7:0]  io_slave_awlen,
  input  [2:0]  io_slave_awsize,
  input  [1:0]  io_slave_awburst,

  output        io_slave_wready,
  input         io_slave_wvalid,
  input  [31:0] io_slave_wdata,
  input  [3:0]  io_slave_wstrb,
  input         io_slave_wlast,

  input         io_slave_bready,
  output        io_slave_bvalid,
  output [1:0]  io_slave_bresp,
  output [3:0]  io_slave_bid,

  output        io_slave_arready,
  input         io_slave_arvalid,
  input  [31:0] io_slave_araddr,
  input  [3:0]  io_slave_arid,
  input  [7:0]  io_slave_arlen,
  input  [2:0]  io_slave_arsize,
  input  [1:0]  io_slave_arburst,

  input         io_slave_rready,
  output        io_slave_rvalid,
  output [1:0]  io_slave_rresp,
  output [31:0] io_slave_rdata,
  output        io_slave_rlast,
  output [3:0]  io_slave_rid,

  // 调试接口
  output [31:0] out_pc,
  output [31:0] out_reg [31:0],
  output [31:0] out_csr [3:0],
  output inst_valid_flag,

  output [1:0] out_top_state, out_ifu_state, out_lsu_write_state, out_lsu_read_state,
  output reg inst_done
);

  // Master 接口处理
  assign axi_if_arb2top.aw.AWREADY = io_master_awready;
  assign io_master_awvalid = axi_if_arb2top.aw.AWVALID;
  assign io_master_awaddr = axi_if_arb2top.aw.AWADDR;
  assign io_master_awid = 4'b0; // 默认为0
  assign io_master_awlen = 8'b0; // 默认长度为1
  assign io_master_awsize = 3'b010; // 默认为4字节
  assign io_master_awburst = 2'b01; // 默认为 INCR

  assign axi_if_arb2top.w.WREADY = io_master_wready;
  assign io_master_wvalid = axi_if_arb2top.w.WVALID;
  assign io_master_wdata = axi_if_arb2top.w.WDATA;
  assign io_master_wstrb = axi_if_arb2top.w.WSTRB;
  assign io_master_wlast = 1'b1; // 每次都是最后一次传输

  assign io_master_bready = axi_if_arb2top.b.BREADY;
  assign axi_if_arb2top.b.BVALID = io_master_bvalid;
  assign axi_if_arb2top.b.BRESP = io_master_bresp;
  // BID 暂时不使用

  assign axi_if_arb2top.ar.ARREADY = io_master_arready;
  assign io_master_arvalid = axi_if_arb2top.ar.ARVALID;
  assign io_master_araddr = axi_if_arb2top.ar.ARADDR;
  assign io_master_arid = 4'b0; // 暂时不使用
  assign io_master_arlen = 8'b0; // 默认长度1
  assign io_master_arsize = 3'b010; // 默认大小4字节
  assign io_master_arburst = 2'b01; // 默认为INCR

  assign io_master_rready = axi_if_arb2top.r.RREADY;
  assign axi_if_arb2top.r.RVALID = io_master_rvalid;
  assign axi_if_arb2top.r.RRESP = io_master_rresp;
  assign axi_if_arb2top.r.RDATA = io_master_rdata;
  // Rlast 暂时不处理
  // Rid 暂时不处理



  // Slave 接口处理 (目前仅处理一下输出的端口，输出0就行)
  assign io_slave_awready = 1'b0;

  assign io_slave_wready = 1'b0;

  assign io_slave_bvalid = 1'b0;
  assign io_slave_bresp = 2'b00;
  assign io_slave_bid = 4'b0;

  assign io_slave_arready = 1'b0;


  // =========== 调试接口实现 ===========
  assign inst_valid_flag = |{add_en, addi_en, lui_en, lw_en, lbu_en, sw_en, sb_en, jalr_en, ebreak_en, auipc_en, jal_en,
                             sub_en, sltiu_en, beq_en, bne_en, sltu_en, xor_en, or_en, sh_en, srai_en, andi_en, sll_en, 
                             and_en, xori_en, bge_en, blt_en, srli_en, bgeu_en, slli_en, bltu_en, sra_en, srl_en, lh_en,
                             lhu_en, lb_en, ori_en, slti_en, slt_en, csrrc_en, csrrs_en, csrrw_en, ecall_en, mret_en};
  
  assign out_top_state = top_state;
  always_ff @(posedge clock) begin
    if (reset) begin
      inst_done <= 1'b0;
    end else begin
      inst_done <= (top_state == WBU_WAIT) & (top_next_state == IFU_WAIT);
    end
  end

  // =========== TOP FSM ===========
  // 目前使用“总控制器”来控制目前的指令执行流程
  enum logic [1:0] {
    IFU_WAIT,
    EXU_WAIT,
    LSU_WAIT,
    WBU_WAIT
  } top_state, top_next_state;

  // 状态转移
  always_ff @(posedge clock) begin
    if(reset) begin
      top_state <= IFU_WAIT;
    end else begin
      top_state <= top_next_state;
    end
  end

  // 辅助控制
  // 需要保证will_done标志位定义为：will_done == 1 
  // 后的的第一个clk上升沿时，对应模块完成本次事务。

  wire ifu_will_done, lsu_will_done;
  wire wbu_will_done = 1'b1; // just for Reg and PC write
  wire exu_will_done = 1'b1; // just for CSR inst
  // 状态转移逻辑
  always_comb begin
    // 默认值
    top_next_state = top_state;

    case(top_state)
    IFU_WAIT: begin
      if(ifu_will_done) begin
        top_next_state = EXU_WAIT;
      end else begin
        top_next_state = IFU_WAIT;
      end
    end

    EXU_WAIT: begin
      if(exu_will_done) begin
        if(is_mem) begin
          top_next_state = LSU_WAIT;
        end else begin
          top_next_state = WBU_WAIT;
        end
      end else begin
        top_next_state = EXU_WAIT;
      end
    end

    LSU_WAIT: begin
      if(lsu_will_done) begin
        top_next_state = WBU_WAIT;
      end else begin
        top_next_state = LSU_WAIT;
      end
    end

    WBU_WAIT: begin
      if(wbu_will_done) begin
        top_next_state = IFU_WAIT;
      end else begin
        top_next_state = WBU_WAIT;
      end
    end
    endcase
  end


  reg ifu_en, exu_en, lsu_en, wbu_en;
	// 使能控制
	always @(posedge clock) begin
		if(reset) begin
			ifu_en <= 1'b0;
			exu_en <= 1'b0;
			lsu_en <= 1'b0;
			wbu_en <= 1'b0;
		end else begin 
			case(top_state)
			IFU_WAIT: begin 
				if(top_next_state == EXU_WAIT) begin
					exu_en <= 1'b1;
					ifu_en <= 1'b0;
				end else begin // 简单复位
					ifu_en <= 1'b1;
					exu_en <= 1'b0; lsu_en <= 1'b0; wbu_en <= 1'b0;
				end
			end

			EXU_WAIT: begin 
				if(top_next_state == LSU_WAIT) begin
					lsu_en <= 1'b1;
					exu_en <= 1'b0;
        end else 
        if(top_next_state == WBU_WAIT) begin
          wbu_en <= 1'b1;
          exu_en <= 1'b0;
        end
			end

			LSU_WAIT: begin 
				if(top_next_state == WBU_WAIT) begin
					wbu_en <= 1'b1;
					lsu_en <= 1'b0;
				end 
			end

			WBU_WAIT: begin 
				if(top_next_state == IFU_WAIT) begin
					ifu_en <= 1'b1;
					wbu_en <= 1'b0;
				end
			end
			endcase
		end
	end


  // =========== IFU例化 ===========
  wire [31:0] inst;
  assign out_pc = pc;

  axi4_lite_if axi_if_lfu2rom();
  assign axi_if_lfu2rom.ACLK = clock;
  assign axi_if_lfu2rom.ARESETn = ~reset;

  wire [1:0] ifu_resp;

  axi4_lite_ifu uAXI4_ifu(
    .axi_if(axi_if_lfu2rom),
    .en(ifu_en),
    .will_done(ifu_will_done),
    .raddr(pc),
    .rresp(ifu_resp),
    .rdata(inst),
    .ifu_state(out_ifu_state)
  );
  
  // =========== PC 寄存器实现 ===========
  wire [31:0] pc;
  wire [31:0] next_pc;

  // WBU 输入 至 PC
  assign next_pc = wb_next_pc;

  ysyx_25090244_PC uPC (
    .clk(clock),
    .rst(reset),
    .next_pc(next_pc),
    .pc(pc),
    .update_en(wb_pc_en)
  );


  // =========== IDU 例化 ===========
  wire [4:0] rs1_addr, rs2_addr, rd_addr;
  wire [31:0] rs1_val, rs2_val, rd_val;
  wire [31:0] imm;
  wire add_en, addi_en, lui_en, lw_en, lbu_en, sw_en, sb_en, jalr_en, ebreak_en, auipc_en, jal_en;
  wire sub_en, sltiu_en, beq_en, bne_en, sltu_en, xor_en, or_en, sh_en, srai_en, andi_en, sll_en;
  wire and_en, xori_en, bge_en, blt_en, srli_en, bgeu_en, slli_en, bltu_en, sra_en, srl_en, lh_en;
  wire lhu_en, lb_en, ori_en, slti_en, slt_en, csrrc_en, csrrs_en, csrrw_en, ecall_en, mret_en;
  wire [11:0] I_imm, S_imm;
  wire [12:0] B_imm;
  wire [31:0] U_imm;
  wire [20:0] J_imm;

  wire is_mem, is_load, is_store;

  ysyx_25090244_IDU uIDU (
    .inst(inst),
    .rs1_addr(rs1_addr),
    .rs2_addr(rs2_addr),
    .rd_addr(rd_addr),

    .add_en(add_en), .addi_en(addi_en), .lui_en(lui_en), .lw_en(lw_en), .lbu_en(lbu_en),
    .sw_en(sw_en), .sb_en(sb_en), .jalr_en(jalr_en), .ebreak_en(ebreak_en), .auipc_en(auipc_en),
    .jal_en(jal_en), .sub_en(sub_en), .sltiu_en(sltiu_en), .beq_en(beq_en), .bne_en(bne_en),
    .sltu_en(sltu_en), .xor_en(xor_en), .or_en(or_en), .sh_en(sh_en), .srai_en(srai_en),
    .andi_en(andi_en), .sll_en(sll_en), .and_en(and_en), .xori_en(xori_en), .bge_en(bge_en),
    .blt_en(blt_en), .srli_en(srli_en), .bgeu_en(bgeu_en), .slli_en(slli_en), .bltu_en(bltu_en),
    .sra_en(sra_en), .srl_en(srl_en), .lh_en(lh_en), .lhu_en(lhu_en), .lb_en(lb_en), .ori_en(ori_en),
    .slti_en(slti_en), .slt_en(slt_en), .csrrc_en(csrrc_en), .csrrs_en(csrrs_en), .csrrw_en(csrrw_en),
    .ecall_en(ecall_en), .mret_en(mret_en),

    .I_imm(I_imm),
    .S_imm(S_imm),
    .B_imm(B_imm),
    .U_imm(U_imm),
    .J_imm(J_imm),

    .is_mem(is_mem),
    .is_load(is_load),
    .is_store(is_store)
  );


  // =========== EXU 例化 ===========
  assign imm =  (addi_en | lw_en | lbu_en | jalr_en | sltiu_en | andi_en | xori_en | lh_en | lhu_en |
                 lb_en | ori_en | slti_en | csrrc_en | csrrs_en | csrrw_en) ? {{20{I_imm[11]}}, I_imm} :
                (lui_en) ? U_imm :
                (sw_en | sb_en | sh_en) ? {{20{S_imm[11]}}, S_imm} :
                (auipc_en) ? U_imm :
                (jal_en) ? {{11{J_imm[20]}}, J_imm} :
                (beq_en | bne_en | bge_en | blt_en | bgeu_en | bltu_en) ? {{19{B_imm[12]}}, B_imm} :
                (srai_en | srli_en | slli_en) ? {27'b0, I_imm[4:0]} : // 特殊立即数（shamt in RV32I）
                32'b0;

  wire [31:0] ex_next_pc;
  wire [31:0] ex_rs1_val, ex_rs2_val;
  wire        ex_ram_we, ex_ram_re, ex_reg_we;
  wire [31:0] ex_ram_write_addr, ex_ram_write_data, ex_ram_read_addr;
  wire [3:0]  ex_ram_write_mask;
  wire [31:0] ex_reg_data;
  wire [4:0]  ex_reg_addr;

  // CSR
  wire ex_csr_we;
  wire [11:0] ex_csr_raddr, ex_csr_waddr;
  wire [31:0] ex_csr_rdata, ex_csr_wdata;

  wire ex_csr_we1;
  wire [11:0] ex_csr_waddr1;
  wire [31:0] ex_csr_wdata1;

  assign ex_csr_rdata = csr_rdata;

  // 寄存器值输入（rs1 rs2 输入）
  assign ex_rs1_val = reg_addr1_val;
  assign ex_rs2_val = reg_addr2_val;

  // 需要写回
  wire need_wb;
  

  ysyx_25090244_EXU uEXU (
    .add_en(add_en), .addi_en(addi_en), .lui_en(lui_en), .lw_en(lw_en), .lbu_en(lbu_en),
    .sw_en(sw_en), .sb_en(sb_en), .jalr_en(jalr_en), .ebreak_en(ebreak_en), .auipc_en(auipc_en),
    .jal_en(jal_en), .sub_en(sub_en), .sltiu_en(sltiu_en), .beq_en(beq_en), .bne_en(bne_en),
    .sltu_en(sltu_en), .xor_en(xor_en), .or_en(or_en), .sh_en(sh_en), .srai_en(srai_en),
    .andi_en(andi_en), .sll_en(sll_en), .and_en(and_en), .xori_en(xori_en), .bge_en(bge_en),
    .blt_en(blt_en), .srli_en(srli_en), .bgeu_en(bgeu_en), .slli_en(slli_en), .bltu_en(bltu_en),
    .sra_en(sra_en), .srl_en(srl_en), .lh_en(lh_en), .lhu_en(lhu_en), .lb_en(lb_en), .ori_en(ori_en),
    .slti_en(slti_en), .slt_en(slt_en), .csrrc_en(csrrc_en), .csrrs_en(csrrs_en), .csrrw_en(csrrw_en),
    .ecall_en(ecall_en), .mret_en(mret_en),

    .rs1_val(ex_rs1_val),
    .rs2_val(ex_rs2_val),
    .rd_addr(rd_addr),
    .ram_read_val(ram_rdata),
    .imm(imm),
    .pc(pc),
    .next_pc(ex_next_pc),

    .ram_we(ex_ram_we),
    .ram_write_addr(ex_ram_write_addr),
    .ram_write_data(ex_ram_write_data),
    .ram_write_mask(ex_ram_write_mask),

    .ram_re(ex_ram_re),
    .ram_read_addr(ex_ram_read_addr),

    .reg_we(ex_reg_we),
    .reg_addr(ex_reg_addr),
    .reg_data(ex_reg_data),

    .csr_we(ex_csr_we),
    .csr_raddr(ex_csr_raddr),
    .csr_rdata(ex_csr_rdata),

    .csr_waddr(ex_csr_waddr),
    .csr_wdata(ex_csr_wdata),

    .csr_we1(ex_csr_we1),
    .csr_waddr1(ex_csr_waddr1),
    .csr_wdata1(ex_csr_wdata1),

    .en(exu_en),
    .need_wb(need_wb)
  );


  // =========== RV_regs 例化 ===========
  wire reg_write_ena;
  wire [4:0] reg_write_addr, reg_read_addr1, reg_read_addr2;

  wire [31:0] reg_write_val, reg_addr1_val, reg_addr2_val;

  // // A0 输出
  // assign A0 = A0_val;
  wire [31:0] A0_val;

  // WBU 输入 至 写端口
  assign reg_write_ena = wb_reg_we;
  assign reg_write_addr = wb_reg_addr;
  assign reg_write_val = wb_reg_data;

  // rs1 rs2 的固定读端口
  assign reg_read_addr1 = rs1_addr;
  assign reg_read_addr2 = rs2_addr;
  
  ysyx_25090244_RV32_regs uRV32_regs (
    .clk(clock),
    .write_ena_in(reg_write_ena),
    .write_addr(reg_write_addr),
    .write_val(reg_write_val),

    .read_addr1(reg_read_addr1),
    .addr1_val(reg_addr1_val),

    .read_addr2(reg_read_addr2),
    .addr2_val(reg_addr2_val),
    .A0_val(A0_val),
    .reg_val(out_reg)
  );


  // ========= RV32_csrs 例化 ========
  wire csr_we, csr_we1;
  wire [31:0] csr_wdata, csr_rdata;
  wire [11:0] csr_raddr, csr_waddr;

  wire [31:0] csr_wdata1;
  wire [11:0] csr_waddr1;

  assign csr_we = ex_csr_we;
  assign csr_wdata = ex_csr_wdata;
  assign csr_raddr = ex_csr_raddr;
  assign csr_waddr = ex_csr_waddr;

  assign csr_we1 = ex_csr_we1;
  assign csr_wdata1 = ex_csr_wdata1;
  assign csr_waddr1 = ex_csr_waddr1;

  ysyx_25090244_RV32_csrs uRV32_csrs (
    .clk(clock),
    .rst(reset),
    
    .we_in(csr_we),
    .waddr(csr_waddr),
    .wdata(csr_wdata),

    .raddr(csr_raddr),
    .rdata(csr_rdata),

    .we1_in(csr_we1),
    .waddr1(csr_waddr1),
    .wdata1(csr_wdata1),
    .out_csr(out_csr)
  );


  // =========== LSU 例化 ===========
  wire [31:0] ram_read_data;

  wire [31:0] ram_rdata;
  wire [1:0] ram_wresp, ram_rresp;

  axi4_lite_if axi_if_lsu2ram();
  assign axi_if_lsu2ram.ACLK = clock;
  assign axi_if_lsu2ram.ARESETn = ~reset;

  axi4_lite_npcside uAXI4_lsu (
    .axi_if(axi_if_lsu2ram),
    .en(lsu_en),
    .will_done(lsu_will_done),

    .is_load(is_load),
    .is_store(is_store),
    .raddr(ex_ram_read_addr),
    .waddr(ex_ram_write_addr),
    .wdata(ex_ram_write_data),
    .wstrb(ex_ram_write_mask),

    .wresp(ram_wresp),
    .rresp(ram_rresp),

    .rdata(ram_rdata),
    .lsu_wirte_state(out_lsu_write_state),
    .lsu_read_state(out_lsu_read_state)
  );


  // =========== WBU 例化 ===========
  wire wb_reg_we;
  wire [4:0] wb_reg_addr;
  wire [31:0] wb_reg_data, wb_next_pc;
  wire wb_pc_en;


  ysyx_25090244_WBU uWBU (
    .next_pc_EX(ex_next_pc),
    .reg_we_EX(ex_reg_we),
    .reg_addr_EX(ex_reg_addr),
    .reg_data_EX(ex_reg_data),

    .reg_we(wb_reg_we),
    .reg_addr(wb_reg_addr),
    .reg_data(wb_reg_data),
    .next_pc(wb_next_pc),

    .pc_en(wb_pc_en),
    .en(wbu_en)
  );

  // ========== Arbiter 例化 ===========
  axi4_lite_if axi_if_arb2top();
  assign axi_if_arb2top.ACLK = clock;
  assign axi_if_arb2top.ARESETn = ~reset;

  Arbiter uArbiter (
    .axi_if_a(axi_if_lfu2rom),
    .axi_if_b(axi_if_lsu2ram),

    .axi_if_out(axi_if_arb2top)
  );


  // =========== CLINT 例化 ===========
  // CLINT uCLINT (
  //   .axi_if(axi_if_xbar2clint)
  // );
endmodule
