package idu
import chisel3._
import chiseltest._
import org.scalatest.flatspec.AnyFlatSpec

import exu._

class IDUTest extends AnyFlatSpec with ChiselScalatestTester {

  "InstDecodeUnit" should "decode R-type ADD correctly" in {
    test(new InstDecodeUnit) { dut =>
      dut.io.inst.poke("h00000033".U) // ADD R-type
      dut.clock.step()
      dut.io.ctrl.aluOp.expect(ALUOp.ADD)
      dut.io.ctrl.branchOp.expect(BranchOp.NONE)
      dut.io.ctrl.csrOp.expect(CsrOp.NONE)
      dut.io.ctrl.sysOp.expect(SysOp.NONE)
      dut.io.ctrl.memOp.expect(MemOp.NONE)
      dut.io.ctrl.memWidth.expect(MemWidth.NONE)
      dut.io.ctrl.src1Sel.expect(Src1Sel.RS1)
      dut.io.ctrl.src2Sel.expect(Src2Sel.RS2)
      dut.io.ctrl.writeBack.expect(true.B)
      dut.io.ctrl.signExtend.expect(SignExtend.NONE)
    }
  }

  it should "decode R-type SUB correctly" in {
    test(new InstDecodeUnit) { dut =>
      dut.io.inst.poke("h40000033".U) // SUB R-type
      dut.clock.step()
      dut.io.ctrl.aluOp.expect(ALUOp.SUB)
    }
  }

  it should "decode I-type ADDI correctly" in {
    test(new InstDecodeUnit) { dut =>
      dut.io.inst.poke("h00100093".U) // ADDI
      dut.clock.step()
      dut.io.ctrl.aluOp.expect(ALUOp.ADD)
    }
  }

  it should "decode load LW correctly" in {
    test(new InstDecodeUnit) { dut =>
      dut.io.inst.poke("h00002283".U) // LW
      dut.clock.step()
      dut.io.ctrl.memOp.expect(MemOp.LOAD)  // 假设 MemOp.LOAD 是你 IDU 定义的
    }
  }
}


