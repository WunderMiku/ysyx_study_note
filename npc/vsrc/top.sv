// =========== 总线实现 ===========
interface simple_bus_IFU();
  logic [31:0] ifu_raddr;
  logic [31:0] ifu_rdata;

  modport IFU_port (
    input ifu_raddr,
    output ifu_rdata
  );

endinterface 

module ysyx_25090244_top (
  input clk,
  input rst,
  output [31:0] A0,

  // Ram 接口
  input reg [31:0] ramReadData,
  output [31:0] ramReadAddr,
  output ramRe,

  output [31:0] ramWriteAddr,
  output [31:0] ramWriteData,
  output [3:0]  ramWriteMask,
  output ramWe,

  // 调试接口
  output [31:0] out_pc,
  output [31:0] out_reg [31:0],
  output [31:0] out_csr [3:0],
  output inst_valid_flag,
  output ifu_valid_flag,

  output [1:0] out_top_state, out_ifu_state, out_lsu_state
);

  // =========== 调试接口实现 ===========
  assign inst_valid_flag = |{add_en, addi_en, lui_en, lw_en, lbu_en, sw_en, sb_en, jalr_en, ebreak_en, auipc_en, jal_en,
                             sub_en, sltiu_en, beq_en, bne_en, sltu_en, xor_en, or_en, sh_en, srai_en, andi_en, sll_en, 
                             and_en, xori_en, bge_en, blt_en, srli_en, bgeu_en, slli_en, bltu_en, sra_en, srl_en, lh_en,
                             lhu_en, lb_en, ori_en, slti_en, slt_en, csrrc_en, csrrs_en, csrrw_en, ecall_en, mret_en};
  
  assign out_top_state = top_state;

  // =========== TOP FSM ===========
  // 目前使用“总控制器”来控制目前的指令执行流程
  enum logic [1:0] {
    IDLE,     // IFU PAUSE  
    MEM_WAIT  // LSU PAUSE
  } top_state, next_top_state;

  always_ff @(posedge clk or posedge rst) begin
    if(rst) begin
      top_state <= IDLE;
    end else begin
      top_state <= next_top_state;
    end
  end


  always_comb begin
  // 默认保持
  next_top_state = top_state;

  case (top_state)
    IDLE: begin
      if(ifu_will_done) begin // ifu 将会发射
        if (lsu_will_done)
          next_top_state = IDLE;
        else
          next_top_state = MEM_WAIT;
      end else begin 
        next_top_state = IDLE; // 保持等待ifu发射
      end
    end

    MEM_WAIT: begin
      if (lsu_will_done)
        next_top_state = IDLE;
      else
        next_top_state = MEM_WAIT;
    end

    default: begin
      next_top_state = IDLE;
    end
  endcase
end

  wire ifu_en, exu_en, lsu_en, wbu_en;
  wire ifu_will_done, lsu_will_done;
  // lsu_will_done:
  // 1: 如果内存操作在本周期发出，结果将在下一周期就绪（或是指令不需要访存）
  // 0: 必须暂停（进入MEM_WAIT状态）

  // ifu_will_done:
  // 1: IFU会在本周期发出，结果将在下一周期就绪
  // 0: IFU将不会发射，需要暂停等待

  assign ifu_en = (top_state == IDLE);
  assign exu_en = ifu_will_done;
  assign lsu_en = ifu_will_done | (top_state == MEM_WAIT);
  assign wbu_en = (is_mem & (top_state == MEM_WAIT) & lsu_will_done) | (~is_mem & ifu_will_done);
  // =========== IFU例化 ===========
  wire [31:0] inst;

  // 总线设置
  simple_bus_IFU ifu_bus();
  assign ifu_bus.ifu_raddr = pc;
  assign inst = ifu_bus.ifu_rdata;

  ysyx_25090244_IFU uIFU (
    .clk(clk),
    .rst(rst),
    .bus_in(ifu_bus),
    .en(ifu_en),
    .will_done(ifu_will_done),

    .out_state(out_ifu_state)
  );

  assign out_pc = pc;
  
  // =========== PC 寄存器实现 ===========
  wire [31:0] pc;
  wire [31:0] next_pc;

  // WBU 输入 至 PC
  assign next_pc = wb_next_pc;

  ysyx_25090244_PC uPC (
    .clk(clk),
    .rst(rst),
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

  wire is_mem;

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

    .is_mem(is_mem)
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
    .ram_read_val(ramReadData),
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

    .en(exu_en)
  );


  // =========== RV_regs 例化 ===========
  wire reg_write_ena;
  wire [4:0] reg_write_addr, reg_read_addr1, reg_read_addr2;

  wire [31:0] reg_write_val, reg_addr1_val, reg_addr2_val;

  // A0 输出
  assign A0 = A0_val;
  wire [31:0] A0_val;

  // WBU 输入 至 写端口
  assign reg_write_ena = wb_reg_we;
  assign reg_write_addr = wb_reg_addr;
  assign reg_write_val = wb_reg_data;

  // rs1 rs2 的固定读端口
  assign reg_read_addr1 = rs1_addr;
  assign reg_read_addr2 = rs2_addr;
  
  ysyx_25090244_RV32_regs uRV32_regs (
    .clk(clk),
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
    .clk(clk),
    .rst(rst),
    
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

  // 临时辅助信号
  wire ram_we_Top;
  wire [31:0] ram_write_data_Top;
  wire [31:0] ram_write_addr_Top;
  wire [3:0]  ram_write_mask_Top;

  wire ram_re_Top;
  wire [31:0] ram_read_addr_Top;
  wire [31:0] ram_read_data_Top;

  ysyx_25090244_LSU uLSU (
    .clk(clk),
    .rst(rst),
    
    .ram_we_EX(ex_ram_we),
    .ram_write_addr_EX(ex_ram_write_addr),
    .ram_write_data_EX(ex_ram_write_data),
    .ram_write_mask_EX(ex_ram_write_mask),

    .ram_re_EX(ex_ram_re),
    .ram_read_addr_EX(ex_ram_read_addr),

    .ram_read_data(ram_read_data),

    // 临时辅助信号
    .ram_we_Top(ram_we_Top),
    .ram_write_data_Top(ram_write_data_Top),
    .ram_write_addr_Top(ram_write_addr_Top),
    .ram_write_mask_Top(ram_write_mask_Top),

    .ram_re_Top(ram_re_Top),
    .ram_read_addr_Top(ram_read_addr_Top),
    .ram_read_data_Top(ram_read_data_Top),

    .en(lsu_en),
    .will_done(lsu_will_done),

    .out_state(out_lsu_state)
  );
  
  // =========== RAM 辅助接口处理 ===========
  assign ramWe = ram_we_Top;
  assign ramWriteData = ram_write_data_Top;
  assign ramWriteAddr = ram_write_addr_Top;
  assign ramWriteMask = ram_write_mask_Top;

  assign ramRe = ram_re_Top;
  assign ramReadAddr = ram_read_addr_Top;
  assign ram_read_data_Top = ramReadData;
  

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
endmodule
