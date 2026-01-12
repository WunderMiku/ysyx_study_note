package top

import chisel3._
import idu._
import exu._

class Top extends Module {
	val io = IO(new Bundle {
	})
	 val IDU = Module(new idu.InstDecodeUnit())
	 val EXU = Module(new exu.EXU())

	 IDU.io := DontCare
	 EXU.io := DontCare
}