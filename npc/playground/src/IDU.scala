package idu

import chisel3._
import chisel3.util.switch
import chisel3.util.is
import chisel3.util.MuxLookup

import inst.{InstDecode, InstType}
import exu.EXUCtrl
import os.write

class InstDecodeUnit extends Module {
  val io = IO(new Bundle {
    val inst     = Input(UInt(32.W))
    val ctrl     = Output(new EXUCtrl)
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
  import exu.{EXUCtrl, ALUOp, SignExtend, MemOp, MemWidth, BranchOp, Src1Sel, Src2Sel, CsrOp, SysOp}

  val exuCtrl = Wire(new EXUCtrl)

  // 初始化默认值
  exuCtrl := 0.U.asTypeOf(new EXUCtrl)

  when(instType === InstType.ADD) {
    exuCtrl.aluOp := ALUOp.ADD
    exuCtrl.writeBack := true.B

  } .elsewhen(instType === InstType.SUB) {
    exuCtrl.aluOp := ALUOp.SUB
    exuCtrl.writeBack := true.B

  } .elsewhen(instType === InstType.ADDI) {
    exuCtrl.aluOp := ALUOp.ADD
    exuCtrl.src2Sel := Src2Sel.IMM
    exuCtrl.writeBack := true.B

  } .elsewhen(instType === InstType.SLT) {
    exuCtrl.signExtend := SignExtend.SIGN
    exuCtrl.branchOp := BranchOp.LESS
    exuCtrl.writeBack := true.B

  } .elsewhen(instType === InstType.SLTU) {
    exuCtrl.signExtend := SignExtend.ZERO
    exuCtrl.branchOp := BranchOp.LESS
    exuCtrl.writeBack := true.B

  } .elsewhen(instType === InstType.SLTI) {
    exuCtrl.signExtend := SignExtend.SIGN
    exuCtrl.src2Sel := Src2Sel.IMM
    exuCtrl.branchOp := BranchOp.LESS
    exuCtrl.writeBack := true.B

  } .elsewhen(instType === InstType.SLTIU) {
    exuCtrl.signExtend := SignExtend.ZERO
    exuCtrl.src2Sel := Src2Sel.IMM
    exuCtrl.branchOp := BranchOp.LESS
    exuCtrl.writeBack := true.B

  } .elsewhen(instType === InstType.XOR) {
    exuCtrl.aluOp := ALUOp.XOR
    exuCtrl.writeBack := true.B

  } .elsewhen(instType === InstType.OR) {
    exuCtrl.aluOp := ALUOp.OR
    exuCtrl.writeBack := true.B

  } .elsewhen(instType === InstType.AND) {
    exuCtrl.aluOp := ALUOp.AND
    exuCtrl.writeBack := true.B

  } .elsewhen(instType === InstType.XORI) { 
    exuCtrl.aluOp := ALUOp.XOR
    exuCtrl.src2Sel := Src2Sel.IMM
    exuCtrl.writeBack := true.B

  } .elsewhen(instType === InstType.ORI) {
    exuCtrl.aluOp := ALUOp.OR
    exuCtrl.src2Sel := Src2Sel.IMM
    exuCtrl.writeBack := true.B

  } .elsewhen(instType === InstType.ANDI) {
    exuCtrl.aluOp := ALUOp.AND
    exuCtrl.src2Sel := Src2Sel.IMM
    exuCtrl.writeBack := true.B

  } .elsewhen(instType === InstType.SLL) {
    exuCtrl.aluOp := ALUOp.SL
    exuCtrl.writeBack := true.B

  } .elsewhen(instType === InstType.SLLI) {
    exuCtrl.aluOp := ALUOp.SL
    exuCtrl.src2Sel := Src2Sel.IMM
    exuCtrl.writeBack := true.B

  } .elsewhen(instType === InstType.SRL) {
    exuCtrl.aluOp := ALUOp.SR
    exuCtrl.signExtend := SignExtend.ZERO
    exuCtrl.writeBack := true.B

  } .elsewhen(instType === InstType.SRLI) {
    exuCtrl.aluOp := ALUOp.SR
    exuCtrl.signExtend := SignExtend.ZERO
    exuCtrl.src2Sel := Src2Sel.IMM
    exuCtrl.writeBack := true.B

  } .elsewhen(instType === InstType.SRA) {
    exuCtrl.aluOp := ALUOp.SR
    exuCtrl.signExtend := SignExtend.SIGN
    exuCtrl.writeBack := true.B

  } .elsewhen(instType === InstType.SRAI) {
    exuCtrl.aluOp := ALUOp.SR
    exuCtrl.src2Sel := Src2Sel.IMM
    exuCtrl.signExtend := SignExtend.SIGN
    exuCtrl.writeBack := true.B

  } .elsewhen(instType === InstType.LB) {
    exuCtrl.aluOp := ALUOp.ADD
    exuCtrl.signExtend := SignExtend.SIGN
    exuCtrl.memOp := MemOp.LOAD
    exuCtrl.memWidth := MemWidth.BYTE
    exuCtrl.src2Sel := Src2Sel.IMM
    exuCtrl.writeBack := true.B

  } .elsewhen(instType === InstType.LBU) {
    exuCtrl.aluOp := ALUOp.ADD
    exuCtrl.signExtend := SignExtend.ZERO
    exuCtrl.memOp := MemOp.LOAD
    exuCtrl.memWidth := MemWidth.BYTE
    exuCtrl.src2Sel := Src2Sel.IMM
    exuCtrl.writeBack := true.B

  } .elsewhen(instType === InstType.LH) {
    exuCtrl.aluOp := ALUOp.ADD
    exuCtrl.signExtend := SignExtend.SIGN
    exuCtrl.memOp := MemOp.LOAD
    exuCtrl.memWidth := MemWidth.HALF
    exuCtrl.src2Sel := Src2Sel.IMM
    exuCtrl.writeBack := true.B

  } .elsewhen(instType === InstType.LHU) {
    exuCtrl.aluOp := ALUOp.ADD
    exuCtrl.signExtend := SignExtend.ZERO
    exuCtrl.memOp := MemOp.LOAD
    exuCtrl.memWidth := MemWidth.HALF
    exuCtrl.src2Sel := Src2Sel.IMM
    exuCtrl.writeBack := true.B

  } .elsewhen(instType === InstType.LW) {
    exuCtrl.aluOp := ALUOp.ADD
    exuCtrl.signExtend := SignExtend.SIGN
    exuCtrl.memOp := MemOp.LOAD
    exuCtrl.memWidth := MemWidth.WORD
    exuCtrl.src2Sel := Src2Sel.IMM
    exuCtrl.writeBack := true.B

  } .elsewhen(instType === InstType.SB) {
    exuCtrl.aluOp := ALUOp.ADD
    exuCtrl.memOp := MemOp.STORE
    exuCtrl.memWidth := MemWidth.BYTE
    exuCtrl.src2Sel := Src2Sel.IMM

  } .elsewhen(instType === InstType.SH) {
    exuCtrl.aluOp := ALUOp.ADD
    exuCtrl.memOp := MemOp.STORE
    exuCtrl.memWidth := MemWidth.HALF
    exuCtrl.src2Sel := Src2Sel.IMM

  } .elsewhen(instType === InstType.SW) {
    exuCtrl.aluOp := ALUOp.ADD
    exuCtrl.memOp := MemOp.STORE
    exuCtrl.memWidth := MemWidth.WORD
    exuCtrl.src2Sel := Src2Sel.IMM

  } .elsewhen(instType === InstType.BEQ) {
    exuCtrl.branchOp := BranchOp.EQ

  } .elsewhen(instType === InstType.BNE) {
    exuCtrl.branchOp := BranchOp.NEQ

  } .elsewhen(instType === InstType.BLT) {
    exuCtrl.signExtend := SignExtend.SIGN
    exuCtrl.branchOp := BranchOp.LESS

  } .elsewhen(instType === InstType.BGE) {
    exuCtrl.signExtend := SignExtend.SIGN
    exuCtrl.branchOp := BranchOp.GEQ

  } .elsewhen(instType === InstType.BLTU) {
    exuCtrl.signExtend := SignExtend.ZERO
    exuCtrl.branchOp := BranchOp.LESS

  } .elsewhen(instType === InstType.BGEU) {
    exuCtrl.signExtend := SignExtend.ZERO
    exuCtrl.branchOp := BranchOp.GEQ

  } .elsewhen(instType === InstType.JAL) {
    exuCtrl.aluOp := ALUOp.JAL
    exuCtrl.src1Sel := Src1Sel.PC
    exuCtrl.src2Sel := Src2Sel.IMM
    exuCtrl.writeBack := true.B

  } .elsewhen(instType === InstType.JALR) {
    exuCtrl.aluOp := ALUOp.JALR
    exuCtrl.src2Sel := Src2Sel.IMM
    exuCtrl.writeBack := true.B

  } .elsewhen(instType === InstType.LUI) {
    exuCtrl.aluOp := ALUOp.LUI
    exuCtrl.src1Sel := Src1Sel.NONE
    exuCtrl.src2Sel := Src2Sel.IMM
    exuCtrl.writeBack := true.B

  } .elsewhen(instType === InstType.AUIPC) {
    exuCtrl.aluOp := ALUOp.AUIPC
    exuCtrl.src1Sel := Src1Sel.PC
    exuCtrl.src2Sel := Src2Sel.IMM
    exuCtrl.writeBack := true.B

  } 
  .elsewhen(instType === InstType.ECALL) { exuCtrl.sysOp := SysOp.ECALL }
  .elsewhen(instType === InstType.EBREAK) { exuCtrl.sysOp := SysOp.EBREAK }
  .elsewhen(instType === InstType.MRET) { exuCtrl.sysOp := SysOp.MRET }
  .elsewhen(instType === InstType.CSRRW) { exuCtrl.csrOp := CsrOp.CSRRW; exuCtrl.writeBack := true.B }
  .elsewhen(instType === InstType.CSRRS) { exuCtrl.csrOp := CsrOp.CSRRS; exuCtrl.writeBack := true.B }
  .elsewhen(instType === InstType.CSRRC) { exuCtrl.csrOp := CsrOp.CSRRC; exuCtrl.writeBack := true.B } 
  .otherwise {
   assert(false.B, "unsupported instruction")
  }

  io.ctrl := exuCtrl
}

