package exu

import chisel3._
import chisel3.util._

import inst.InstType
import port.{ReadPort, WritePort}

class EXUData extends Bundle {
	val rs1Value = UInt(32.W)
	val rs2Value = UInt(32.W)
	val rdAddress = UInt(5.W)
	val imm = UInt(32.W)
}

object ALUOp extends ChiselEnum {
	val ADD, SUB, AND, OR, XOR, SLL, SRL, SRA = Value
}

object Src1Sel extends ChiselEnum {
	val RS1, PC = Value
}

object Src2Sel extends ChiselEnum {
	val RS2, IMM = Value
}

class EXUCtrl extends Bundle {
	val aluOp = ALUOp()
	val src1Sel = Src1Sel()
	val src2Sel = Src2Sel()
	val writeBack = Bool()

	val regWe = Bool()
}

class EXU extends Module {
	val io = IO(new Bundle {
		val inputData = Input(new EXUData)
		
		// PC 相关
		val pc = Input(UInt(32.W))
		val nextPC = Output(UInt(32.W))

		// 内存端口
		val ramWritePort = Flipped(new WritePort)
		val ramReadPort = Flipped(new ReadPort)

		// 寄存器端口
		val regWritePort = Flipped(new WritePort)

		// CSR 端口
		val csrReadPort = Flipped(new ReadPort)
		val csrWritePort1 = Flipped(new WritePort(12, 32))
		val csrWritePort2 = Flipped(new WritePort(12, 32))
	})

	// ===== 寄存器控制 =====
	// 寄存器写端口

}



