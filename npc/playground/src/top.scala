package top

import chisel3._
import idu._
import exu._

class Top extends Module {
  val io = IO(new Bundle {
    val inst = Input(UInt(32.W))
    val ctrl = Output(new EXUCtrl)
  })

  val IDU = Module(new idu.InstDecodeUnit())

  IDU.io.inst := io.inst
  io.ctrl     := IDU.io.ctrl
}
