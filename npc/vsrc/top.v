module top (
  input clk,
  input rst,
  input  [31:0] inst,
  output [31:0] pc_out
);
  // PC 实现
  reg  [31:0] pc;
  assign pc_out = pc;

  wire [31:0] next_pc;

  always @(posedge clk) begin
    if (rst) begin
      pc <= 32'h00000000;
    end else begin
      pc <= next_pc;
    end
  end

  // IDU 例化
  wire [4:0] rs1_addr, rs2_addr, rd_addr;
  wire [31:0] rs1_val, rs2_val, rd_val;
  wire [31:0] imm;
  wire add_en, addi_en, lui_en, lw_en, lbu_en, sw_en, sb_en, jalr_en;
  wire [11:0] I_imm, S_imm;
  wire [12:0] B_imm;
  wire [31:0] U_imm;
  wire [20:0] J_imm;
  IDU uIDU (
    .inst(inst),
    .rs1_addr(rs1_addr),
    .rs2_addr(rs2_addr),
    .rd_addr(rd_addr),
    .add_en(add_en),
    .addi_en(addi_en),
    .lui_en(lui_en),
    .lw_en(lw_en),
    .lbu_en(lbu_en),
    .sw_en(sw_en),
    .sb_en(sb_en),
    .jalr_en(jalr_en),
    .I_imm(I_imm),
    .S_imm(S_imm),
    .B_imm(B_imm),
    .U_imm(U_imm),
    .J_imm(J_imm)
  );

  // RV_regs 例化
  wire reg_write_ena;
  wire [4:0] reg_write_addr, reg_read_addr1, reg_read_addr2;

  wire [31:0] reg_write_val, reg_addr1_val, reg_addr2_val;

  // EXU 输入
  assign reg_write_ena = ex_reg_we;
  assign reg_write_addr = ex_reg_addr;
  assign reg_write_val = ex_reg_data;

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
    .addr2_val(reg_addr2_val)
  );

  // EXU 例化
  assign imm =  (addi_en | lw_en | lbu_en | jalr_en) ? {{20{I_imm[11]}}, I_imm} :
                (lui_en) ? U_imm :
                (sw_en | sb_en) ? {{20{S_imm[11]}}, S_imm} :
                32'b0;

  wire [31:0] ex_rs1_val, ex_rs2_val;
  wire ex_ram_we, ex_reg_we;
  wire [31:0] ex_ram_addr, ex_ram_data, ex_reg_data;
  wire [4:0] ex_reg_addr;

  // 寄存器值输入
  assign ex_rs1_val = reg_addr1_val;
  assign ex_rs2_val = reg_addr2_val;

  EXU uEXU (
    .add_en(add_en),
    .addi_en(addi_en),
    .lui_en(lui_en),
    .lw_en(lw_en),
    .lbu_en(lbu_en),
    .sw_en(sw_en),
    .sb_en(sb_en),
    .jalr_en(jalr_en),

    .rs1_val(ex_rs1_val),
    .rs2_val(ex_rs2_val),
    .rd_addr(rd_addr),
    .imm(imm),
    .pc(pc),
    .next_pc(next_pc),

    .ram_we(ex_ram_we),
    .ram_addr(ex_ram_addr),
    .ram_data(ex_ram_data),

    .reg_we(ex_reg_we),
    .reg_addr(ex_reg_addr),
    .reg_data(ex_reg_data)
  );

endmodule