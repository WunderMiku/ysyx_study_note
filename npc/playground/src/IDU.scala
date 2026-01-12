package idu

import chisel3._
import chisel3.util.switch
import chisel3.util.is

import inst.{InstDecode, InstType}

class InstDecodeUnit extends Module {
  val io = IO(new Bundle {
    val inst     = Input(UInt(32.W))
  })

  val instDecode = Wire(new InstDecode)
  instDecode.opcode := io.inst(6, 0)
  instDecode.rd     := io.inst(11, 7)
  instDecode.funct3 := io.inst(14, 12)
  instDecode.rs1    := io.inst(19, 15)
  instDecode.rs2    := io.inst(24, 20)
  instDecode.funct7 := io.inst(31, 25)

  val instType = Wire(InstType())
  instType := InstType.NULL  // 默认NULL

  switch(instDecode.opcode) {
    // =================== R-type ===================
    is("b0110011".U) {
      switch(instDecode.funct3) {
        is(0.U) { // add/sub
          when(instDecode.funct7 === 0.U) { instType := InstType.ADD }
          .elsewhen(instDecode.funct7 === "b0100000".U) { instType := InstType.SUB }
        }
        is(1.U) { instType := InstType.SLL }
        is(2.U) { instType := InstType.SLT }
        is(3.U) { instType := InstType.SLTU }
        is(4.U) { instType := InstType.XOR }
        is(5.U) {
          when(instDecode.funct7 === 0.U) { instType := InstType.SRL }
          .elsewhen(instDecode.funct7 === "b0100000".U) { instType := InstType.SRA }
        }
        is(6.U) { instType := InstType.OR }
        is(7.U) { instType := InstType.AND }
      }
    }

    // =================== I-type ===================
    is("b0010011".U) {
      switch(instDecode.funct3) {
        is(0.U) { instType := InstType.ADDI }
        is(1.U) { instType := InstType.SLLI }
        is(2.U) { instType := InstType.SLTI }
        is(3.U) { instType := InstType.SLTU }
        is(4.U) { instType := InstType.XORI }
        is(5.U) {
          when(instDecode.funct7 === 0.U) { instType := InstType.SRLI }
          .elsewhen(instDecode.funct7 === "b0100000".U) { instType := InstType.SRAI }
        }
        is(6.U) { instType := InstType.ORI }
        is(7.U) { instType := InstType.ANDI }
      }
    }

    is("b0000011".U) {
      switch(instDecode.funct3) {
        is(0.U) { instType := InstType.LB }
        is(1.U) { instType := InstType.LH }
        is(2.U) { instType := InstType.LW }
        is(4.U) { instType := InstType.LBU }
        is(5.U) { instType := InstType.LHU }
      }
    }

    // =================== S-type ===================
    is("b0100011".U) {
      switch(instDecode.funct3) {
        is(0.U) { instType := InstType.SB }
        is(1.U) { instType := InstType.SH }
        is(2.U) { instType := InstType.SW }
      }
    }

    // =================== B-type ===================
    is("b1100011".U) {
      switch(instDecode.funct3) {
        is(0.U) { instType := InstType.BEQ }
        is(1.U) { instType := InstType.BNE }
        is(4.U) { instType := InstType.BLT }
        is(5.U) { instType := InstType.BGE }
        is(6.U) { instType := InstType.BLTU }
        is(7.U) { instType := InstType.BGEU }
      }
    }

    // =================== U-type ===================
    is("b0110111".U) { instType := InstType.LUI }
    is("b0010111".U) { instType := InstType.AUIPC }

    // =================== J-type ===================
    is("b1101111".U) { instType := InstType.JAL }
    is("b1100111".U) {
      when(instDecode.funct3 === 0.U) { instType := InstType.JALR }
    }

    // =================== CSR / 系统 ===================
    is("b1110011".U) {
      switch(instDecode.funct3) {
        is(1.U) { instType := InstType.CSRRW }
        is(2.U) { instType := InstType.CSRRS }
        is(3.U) { instType := InstType.CSRRC }
      }
      when(instDecode.asUInt === "h00000073".U) { instType := InstType.ECALL }
      when(instDecode.asUInt === "h00100073".U) { instType := InstType.EBREAK }
      when(instDecode.asUInt === "h30200073".U) { instType := InstType.MRET }
    }
  }

  // 不支持的指令检测 (暂不使用)
  // assert(instType =/= InstType.NULL, "Unsupported instruction detected in IDU!")

  // ===== EXU 控制信号生成 =====
  val exuCtrl = Wire(new exu.EXUCtrl)

  exuCtrl.writeBack := instType.isOneOf(
    InstType.ADD,
    InstType.ADDI,
    InstType.JALR,
    InstType.LUI,
    InstType.LBU,
    InstType.LW,
    InstType.AUIPC,
    InstType.JAL,
    InstType.SUB,
    InstType.SLTIU,
    InstType.SLTU,
    InstType.XOR,
    InstType.OR,
    InstType.SRAI,
    InstType.ANDI,
    InstType.SLL,
    InstType.AND,
    InstType.XORI,
    InstType.SRLI,
    InstType.SLLI,
    InstType.SRA,
    InstType.SRL,
    InstType.LH,
    InstType.LHU,
    InstType.LB,
    InstType.ORI,
    InstType.SLTI,
    InstType.SLT,
    InstType.CSRRC,
    InstType.CSRRS,
    InstType.CSRRW
  ) && (instDecode.rd =/= 0.U)

  // TODO ALU 操作类型生成
  // when(instType.isOneOf(InstType.ADD, InstType.ADDI, InstType.JALR, InstType.AUIPC, InstType.JAL,
  //  InstType.BEQ, InstType.BNE, InstType.BGE, InstType.BLT, InstType.BLTU, InstType.BGEU, InstType.SB,
  //   InstType.SW, InstType.SH, InstType.LH, InstType.LBU, InstType.LW, InstType.LH, InstType.LHU, InstType.LB)) {
  //   exuCtrl.aluOp := exu.ALUOp.ADD
  // } .elsewhen(instType.isOneOf(InstType.SRAI, InstType.SRA)) {
  //   exuCtrl.aluOp := exu.ALUOp.SRA
  // } .elsewhen(instType.isOneOf(InstType.SRLI, InstType.SRL)) {
  //   exuCtrl.aluOp := exu.ALUOp.SRL
  // } .otherwise {
  // }
  
  // when(instType.isOneOf(InstType.ADD, InstType.SUB, InstType.SLL, InstType.SLT, InstType.SLTU, InstType.XOR, InstType.SRL, InstType.SRA, InstType.OR, InstType.AND)) {
  //   exuCtrl.src2Sel := exu.Src2Sel.RS2
  // } .otherwise {
  //   exuCtrl.src2Sel := exu.Src2Sel.IMM
  // }


}

