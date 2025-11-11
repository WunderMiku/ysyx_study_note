module top (
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
  output inst_valid_flag
);
  // =========== 调试接口实现 ===========
  assign inst_valid_flag = |{add_en, addi_en, lui_en, lw_en, lbu_en, sw_en, sb_en, jalr_en, ebreak_en, auipc_en, jal_en,
                             sub_en, sltiu_en, beq_en, bne_en, sltu_en, xor_en, or_en, sh_en, srai_en, andi_en, sll_en, 
                             and_en, xori_en, bge_en, blt_en, srli_en, bgeu_en, slli_en, bltu_en, sra_en, srl_en, lh_en,
                             lhu_en, lb_en, ori_en, slti_en, slt_en, csrrc_en, csrrs_en, csrrw_en, ecall_en, mret_en};

  // =========== IFU实现 ===========
  import "DPI-C" function int pmem_read(input int raddr, input int len);
  import "DPI-C" function void pmem_write(input int waddr, input int wdata, input byte wmask);

  reg [31:0] inst;
  always @(*) begin
    if(!rst) begin
    inst = pmem_read(pc, 4);
    end else begin
      inst = 32'b0;
    end
  end

  assign out_pc = pc;
  
  // =========== PC 寄存器实现 ===========
  reg  [31:0] pc;
  wire [31:0] next_pc;

  // WBU 输入 至 PC
  assign next_pc = wb_next_pc;

  always @(posedge clk) begin
    if (rst) begin
      pc <= 32'h80000000;
    end else begin
      pc <= next_pc;
    end
  end


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
  IDU uIDU (
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
    .J_imm(J_imm)
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

  EXU uEXU (
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
    .csr_wdata1(ex_csr_wdata1)
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
  
  RV32_regs uRV32_regs (
    .clk(clk),
    .write_ena(reg_write_ena),
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

  assign csr_we = wb_csr_we;
  assign csr_wdata = wb_csr_wdata;
  assign csr_raddr = wb_csr_raddr;
  assign csr_waddr = wb_csr_waddr;

  assign csr_we1 = wb_csr_we1;
  assign csr_wdata1 = wb_csr_wdata1;
  assign csr_waddr1 = wb_csr_waddr1;

  RV32_csrs uRV32_csrs (
    .clk(clk),
    .rst(rst),
    
    .we(csr_we),
    .waddr(csr_waddr),
    .wdata(csr_wdata),

    .raddr(csr_raddr),
    .rdata(csr_rdata),

    .we1(csr_we1),
    .waddr1(csr_waddr1),
    .wdata1(csr_wdata1),
    .out_csr(out_csr)
  );


  // =========== WBU 例化 ===========
  wire wb_reg_we;
  wire [4:0] wb_reg_addr;
  wire [31:0] wb_reg_data, wb_next_pc;

  wire wb_csr_we;
  wire [11:0] wb_csr_raddr, wb_csr_waddr;
  wire [31:0] wb_csr_wdata;

  wire wb_csr_we1;
  wire [11:0] wb_csr_waddr1;
  wire [31:0] wb_csr_wdata1;

  WBU uWBU (
    .next_pc_EX(ex_next_pc),
    .reg_we_EX(ex_reg_we),
    .reg_addr_EX(ex_reg_addr),
    .reg_data_EX(ex_reg_data),

    .csr_we_EX(ex_csr_we),
    .csr_raddr_EX(ex_csr_raddr),

    .csr_waddr_EX(ex_csr_waddr),
    .csr_wdata_EX(ex_csr_wdata),

    .csr_we1_EX(ex_csr_we1),
    .csr_waddr1_EX(ex_csr_waddr1),
    .csr_wdata1_EX(ex_csr_wdata1),

    .reg_we(wb_reg_we),
    .reg_addr(wb_reg_addr),
    .reg_data(wb_reg_data),
    .next_pc(wb_next_pc),

    .csr_we(wb_csr_we),
    .csr_raddr(wb_csr_raddr),

    .csr_waddr(wb_csr_waddr),
    .csr_wdata(wb_csr_wdata),

    .csr_we1(wb_csr_we1),
    .csr_waddr1(wb_csr_waddr1),
    .csr_wdata1(wb_csr_wdata1)
  );


  // =========== LSU 例化 ===========
  wire ls_ram_we;
  wire [31:0] ls_ram_write_addr, ls_ram_write_data;
  wire [3:0] ls_ram_write_mask;

  wire ls_ram_re;
  wire [31:0] ls_ram_read_addr;

  LSU uLSU (
    .ram_we_EX(ex_ram_we),
    .ram_write_addr_EX(ex_ram_write_addr),
    .ram_write_data_EX(ex_ram_write_data),
    .ram_write_mask_EX(ex_ram_write_mask),

    .ram_re_EX(ex_ram_re),
    .ram_read_addr_EX(ex_ram_read_addr),

    .ram_we(ls_ram_we),
    .ram_write_addr(ls_ram_write_addr),
    .ram_write_data(ls_ram_write_data),
    .ram_write_mask(ls_ram_write_mask),

    .ram_re(ls_ram_re),
    .ram_read_addr(ls_ram_read_addr)
  );
  
  // =========== RAM 接口处理 ===========
  assign ramReadAddr = ls_ram_read_addr;
  assign ramRe = ls_ram_re;

  assign ramWriteAddr = ls_ram_write_addr;
  assign ramWriteData = ls_ram_write_data;
  assign ramWriteMask = ls_ram_write_mask;
  assign ramWe = ls_ram_we;
  
endmodule