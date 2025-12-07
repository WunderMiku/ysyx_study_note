package idu

import chisel3._
import chisel3.util.switch
import chisel3.util.is

class InstEnable extends Bundle {
  val add    = Bool()
  val addi   = Bool()
  val lui    = Bool()
  val lw     = Bool()
  val lbu    = Bool()
  val sw     = Bool()
  val sb     = Bool()
  val jalr   = Bool()
  val ebreak = Bool()
  val auipc  = Bool()
  val jal    = Bool()
  val sub    = Bool()
  val sltiu  = Bool()
  val beq    = Bool()
  val bne    = Bool()
  val sltu   = Bool()
  val xor    = Bool()
  val or     = Bool()
  val sh     = Bool()
  val srai   = Bool()
  val andi   = Bool()
  val sll    = Bool()
  val and    = Bool()
  val xori   = Bool()
  val bge    = Bool()
  val blt    = Bool()
  val srli   = Bool()
  val bgeu   = Bool()
  val slli   = Bool()
  val bltu   = Bool()
  val sra    = Bool()
  val srl    = Bool()
  val lh     = Bool()
  val lhu    = Bool()
  val lb     = Bool()
  val ori    = Bool()
  val slti   = Bool()
  val slt    = Bool()
  val csrrc  = Bool()
  val csrrs  = Bool()
  val csrrw  = Bool()
  val ecall  = Bool()
  val mret   = Bool()
}

class InstDecode extends Bundle {
  val opcode = UInt(7.W)
  val rd     = UInt(5.W)
  val rs1    = UInt(5.W)
  val rs2    = UInt(5.W)
  val funct3 = UInt(3.W)
  val funct7 = UInt(7.W)
}

class InstDecodeUnit extends Module {
  val io = IO(new Bundle {
    val instEnable = Output(new InstEnable)
    val inst     = Input(UInt(32.W))
  })

  val instDecode = Wire(new InstDecode)
  instDecode.opcode := io.inst(6, 0)
  instDecode.rd     := io.inst(11, 7)
  instDecode.funct3 := io.inst(14, 12)
  instDecode.rs1    := io.inst(19, 15)
  instDecode.rs2    := io.inst(24, 20)
  instDecode.funct7 := io.inst(31, 25)

  val instEnable = Wire(new InstEnable)

  instEnable := 0.U.asTypeOf(new InstEnable) // 先清零

  switch(instDecode.opcode) {
    // =================== R-type ===================
    is("b0110011".U) {
      switch(instDecode.funct3) {
        is(0.U) { // add/sub
          when(instDecode.funct7 === 0.U) { instEnable.add := true.B }
          .elsewhen(instDecode.funct7 === "b0100000".U) { instEnable.sub := true.B }
        }
        is(1.U) { instEnable.sll := true.B }
        is(2.U) { instEnable.slt := true.B }
        is(3.U) { instEnable.sltu := true.B }
        is(4.U) { instEnable.xor := true.B }
        is(5.U) {
          when(instDecode.funct7 === 0.U) { instEnable.srl := true.B }
          .elsewhen(instDecode.funct7 === "b0100000".U) { instEnable.sra := true.B }
        }
        is(6.U) { instEnable.or := true.B }
        is(7.U) { instEnable.and := true.B }
      }
    }

    // =================== I-type ===================
    is("b0010011".U) {
      switch(instDecode.funct3) {
        is(0.U) { instEnable.addi := true.B }
        is(1.U) { instEnable.slli := true.B }
        is(2.U) { instEnable.slti := true.B }
        is(3.U) { instEnable.sltiu := true.B }
        is(4.U) { instEnable.xori := true.B }
        is(5.U) {
          when(instDecode.funct7 === 0.U) { instEnable.srli := true.B }
          .elsewhen(instDecode.funct7 === "b0100000".U) { instEnable.srai := true.B }
        }
        is(6.U) { instEnable.ori := true.B }
        is(7.U) { instEnable.andi := true.B }
      }
    }

    is("b0000011".U) {
      switch(instDecode.funct3) {
        is(0.U) { instEnable.lb := true.B }
        is(1.U) { instEnable.lh := true.B }
        is(2.U) { instEnable.lw := true.B }
        is(4.U) { instEnable.lbu := true.B }
        is(5.U) { instEnable.lhu := true.B }
      }
    }

    // =================== S-type ===================
    is("b0100011".U) {
      switch(instDecode.funct3) {
        is(0.U) { instEnable.sb := true.B }
        is(1.U) { instEnable.sh := true.B }
        is(2.U) { instEnable.sw := true.B }
      }
    }

    // =================== B-type ===================
    is("b1100011".U) {
      switch(instDecode.funct3) {
        is(0.U) { instEnable.beq := true.B }
        is(1.U) { instEnable.bne := true.B }
        is(4.U) { instEnable.blt := true.B }
        is(5.U) { instEnable.bge := true.B }
        is(6.U) { instEnable.bltu := true.B }
        is(7.U) { instEnable.bgeu := true.B }
      }
    }

    // =================== U-type ===================
    is("b0110111".U) { instEnable.lui := true.B }
    is("b0010111".U) { instEnable.auipc := true.B }

    // =================== J-type ===================
    is("b1101111".U) { instEnable.jal := true.B }
    is("b1100111".U) {
      when(instDecode.funct3 === 0.U) { instEnable.jalr := true.B }
    }

    // =================== CSR / 系统 ===================
    is("b1110011".U) {
      switch(instDecode.funct3) {
        is(1.U) { instEnable.csrrw := true.B }
        is(2.U) { instEnable.csrrs := true.B }
        is(3.U) { instEnable.csrrc := true.B }
      }
      when(instDecode.asUInt === "h00000073".U) { instEnable.ecall := true.B }
      when(instDecode.asUInt === "h00100073".U) { instEnable.ebreak := true.B }
      when(instDecode.asUInt === "h30200073".U) { instEnable.mret := true.B }
    }
  }
  io.instEnable := instEnable
}


