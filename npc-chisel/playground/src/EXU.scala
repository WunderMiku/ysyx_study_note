package exu

import chisel3._
import chisel3.util._

import inst.InstType
import port.{ReadPort, WritePort}
import scribe.ANSI.ctrl

class EXUData extends Bundle {
	val rs1Value = UInt(32.W)
	val rs2Value = UInt(32.W)
	val rdAddress = UInt(5.W)
	val imm = SInt(32.W)
}

object ALUOp extends ChiselEnum {
	val NONE, ADD, SUB, AND, OR, XOR, ShiftLeft, ShiftRight, LESS,
	// 特殊指令
	LUI, AUIPC
	= Value
}

object SignExtend extends ChiselEnum {
	val NONE, SIGN, ZERO = Value
}

object MemOp extends ChiselEnum {
	val NONE, LOAD, STORE = Value
}

object MemWidth extends ChiselEnum {
	val NONE, BYTE, HALF, WORD = Value
}

object BranchOp extends ChiselEnum {
	val NONE, EQ, NEQ, LESS, GEQ, JAL, JALR = Value
}

object Src1Sel extends ChiselEnum {
	val RS1, PC, NONE = Value
}

object Src2Sel extends ChiselEnum {
	val RS2, IMM = Value
}

object CsrOp extends ChiselEnum {
	val NONE, CSRRW, CSRRS, CSRRC = Value
}

object SysOp extends ChiselEnum {
	val NONE, ECALL, EBREAK, MRET = Value
}

class EXUCtrl extends Bundle {
	val aluOp = ALUOp()
	val signExtend = SignExtend()
	val memOp = MemOp()
	val memWidth = MemWidth()
	val branchOp = BranchOp()
	val src1Sel = Src1Sel()
	val src2Sel = Src2Sel()
	val writeBack = Bool()

	val csrOp = CsrOp()
	val sysOp = SysOp()
}

class EXU extends Module {
	val io = IO(new Bundle {
		val inputData = Input(new EXUData)
		val ctrl = Input(new EXUCtrl)
		
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
	// TODO：端口默认值设置
	
	val aluResult = Wire(UInt(32.W))
	val src1Value = Wire(UInt(32.W))
	val src2Value = Wire(UInt(32.W))

	// 选择 ALU 源操作数 1
	switch(io.ctrl.src1Sel) {
		is(Src1Sel.RS1) { src1Value := io.inputData.rs1Value }
		is(Src1Sel.PC)  { src1Value := io.pc }
		is(Src1Sel.NONE){ src1Value := 0.U }
	}
	// 选择 ALU 源操作数 2
	switch(io.ctrl.src2Sel) {
		is(Src2Sel.RS2) { src2Value := io.inputData.rs2Value }
		is(Src2Sel.IMM) { src2Value := io.inputData.imm }
	}
	// ALU 操作
	when(io.ctrl.aluOp === ALUOp.ADD) { aluResult := src1Value + src2Value }
	.elsewhen(io.ctrl.aluOp === ALUOp.SUB) { aluResult := src1Value - src2Value }
	.elsewhen(io.ctrl.aluOp === ALUOp.AND) { aluResult := src1Value & src2Value }
	.elsewhen(io.ctrl.aluOp === ALUOp.OR)  { aluResult := src1Value | src2Value }
	.elsewhen(io.ctrl.aluOp === ALUOp.XOR) { aluResult := src1Value ^ src2Value }
	.elsewhen(io.ctrl.aluOp === ALUOp.ShiftLeft) { aluResult := src1Value << src2Value(4,0) }
	.elsewhen(io.ctrl.aluOp === ALUOp.ShiftRight) {
		when(io.ctrl.signExtend === SignExtend.SIGN) { aluResult := (src1Value.asSInt >> src2Value(4,0)).asUInt }
		.otherwise { aluResult := src1Value >> src2Value(4,0) }
	}
	.elsewhen(io.ctrl.aluOp === ALUOp.LESS) {
		when(io.ctrl.signExtend === SignExtend.SIGN) { aluResult := (src1Value.asSInt < src2Value.asSInt).asUInt }
		.otherwise { aluResult := (src1Value < src2Value).asUInt }
	}
	.elsewhen(io.ctrl.aluOp === ALUOp.LUI) { aluResult := src2Value << 12 }
	.elsewhen(io.ctrl.aluOp === ALUOp.AUIPC) { aluResult := src1Value + (src2Value << 12) }
	// 分支地址计算
	.elsewhen(io.ctrl.branchOp =/= BranchOp.NONE) { aluResult := src1Value + src2Value } 
	.otherwise { aluResult := 0.U }	

	// 分支计算
	when(io.ctrl.branchOp === BranchOp.NONE)
	{ io.nextPC := io.pc + 4.U }

	.elsewhen(io.ctrl.branchOp === BranchOp.EQ) 
	{ io.nextPC := Mux(src1Value === src2Value, aluResult, io.pc + 4.U)}

	.elsewhen(io.ctrl.branchOp === BranchOp.NEQ) 
	{ io.nextPC := Mux(src1Value =/= src2Value, aluResult, io.pc + 4.U)}

	.elsewhen(io.ctrl.branchOp === BranchOp.LESS) 
	{when(io.ctrl.signExtend === SignExtend.SIGN) {
		io.nextPC := Mux(src1Value.asSInt < src2Value.asSInt, aluResult, io.pc + 4.U)
	} .otherwise {
		io.nextPC := Mux(src1Value < src2Value, aluResult, io.pc + 4.U)
	}}

	.elsewhen(io.ctrl.branchOp === BranchOp.GEQ) {
		when(io.ctrl.signExtend === SignExtend.SIGN) {
			io.nextPC := Mux(src1Value.asSInt >= src2Value.asSInt, aluResult, io.pc + 4.U)
		} .otherwise {
			io.nextPC := Mux(src1Value >= src2Value, aluResult, io.pc + 4.U)
		}
	}
	.elsewhen(io.ctrl.branchOp === BranchOp.JAL) {
		io.nextPC := aluResult
	}
	.elsewhen(io.ctrl.branchOp === BranchOp.JALR) {
		io.nextPC := aluResult & Cat(Fill(31, 1.U), 0.U(1.W))
	}

	// 写回数据计算
	val writeBackData = Wire(UInt(32.W))
	// 跳转指令写回 PC+4
	when(io.ctrl.branchOp === BranchOp.JAL || io.ctrl.branchOp === BranchOp.JALR) { writeBackData := io.pc + 4.U }
	// 读数据写回
	.elsewhen(io.ctrl.memOp === MemOp.LOAD){ 
		when(io.ctrl.memWidth === MemWidth.BYTE) { writeBackData := io.ramReadPort.data(7,0)}
		.elsewhen(io.ctrl.memWidth === MemWidth.HALF) { writeBackData := io.ramReadPort.data(15,0)} 
		.elsewhen(io.ctrl.memWidth === MemWidth.WORD) { writeBackData := io.ramReadPort.data } 
	}
	// CSR读取写回
	.elsewhen(io.ctrl.csrOp =/= CsrOp.NONE) { writeBackData := io.csrReadPort.data }
	// 运算结果写回
	.otherwise { writeBackData := aluResult }

	when(io.ctrl.writeBack) {
		io.regWritePort.enable := true.B
		io.regWritePort.addr := io.inputData.rdAddress
		io.regWritePort.data := writeBackData
		io.regWritePort.mask := "b1111".U	
	} .otherwise {
		io.regWritePort.enable := false.B
		io.regWritePort.addr := 0.U
		io.regWritePort.data := 0.U
		io.regWritePort.mask := 0.U
	}

	// 读内存操作
	when(io.ctrl.memOp === MemOp.LOAD) {
		io.ramReadPort.enable := true.B
		io.ramReadPort.addr := aluResult
	} 

	// 写内存操作
	when(io.ctrl.memOp === MemOp.STORE) {
		io.ramWritePort.enable := true.B
		io.ramWritePort.addr := aluResult
		io.ramWritePort.data := io.inputData.rs2Value

		when(io.ctrl.memWidth === MemWidth.BYTE) {
			io.ramWritePort.mask := "b0001".U
		} .elsewhen(io.ctrl.memWidth === MemWidth.HALF) {
			io.ramWritePort.mask := "b0011".U
		} .elsewhen(io.ctrl.memWidth === MemWidth.WORD) {
			io.ramWritePort.mask := "b1111".U
		}
	}

	//CSR 操作
	when(io.ctrl.csrOp =/= CsrOp.NONE) {
		switch(io.ctrl.csrOp) { 
			is(CsrOp.CSRRW) {
				// 读 CSR
				io.csrReadPort.enable := true.B
				io.csrReadPort.addr := io.inputData.imm(11,0)

				// 写 CSR
				io.csrWritePort1.enable := true.B
				io.csrWritePort1.addr := io.inputData.imm(11,0)
				io.csrWritePort1.data := io.inputData.rs1Value
				io.csrWritePort1.mask := "b1111".U

				
			}
			is(CsrOp.CSRRS) {
				io.csrWritePort2.enable := true.B
				io.csrWritePort2.addr := aluResult(11,0)
				io.csrWritePort2.data := io.csrReadPort.data | io.inputData.rs1Value
			}
			is(CsrOp.CSRRC) {
				io.csrWritePort2.enable := true.B
				io.csrWritePort2.addr := aluResult(11,0)
				io.csrWritePort2.data := io.csrReadPort.data & (~io.inputData.rs1Value)
			}
		}
	}

}


