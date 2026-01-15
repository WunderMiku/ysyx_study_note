package inst

import chisel3._

object InstType extends ChiselEnum {
  val NULL, ADD, ADDI, LUI,
      LW, LBU, LH, LHU, LB,
      SW, SB, SH,
      JAL, JALR,
      AUIPC,
      SUB,
      SLTIU, SLTI, SLTU, SLT,
      XOR, OR, AND,
      XORI, ORI, ANDI,
      SLL, SLLI,
      SRL, SRLI,
      SRA, SRAI,
      BEQ, BNE, BLT, BGE, BLTU, BGEU,
      CSRRW, CSRRS, CSRRC,
      ECALL, EBREAK, MRET
      = Value
}


class InstDecode extends Bundle {
  val opcode = UInt(7.W)
  val rd     = UInt(5.W)
  val rs1    = UInt(5.W)
  val rs2    = UInt(5.W)
  val funct3 = UInt(3.W)
  val funct7 = UInt(7.W)
}
