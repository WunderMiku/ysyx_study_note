package port

import chisel3._

class ReadPort (val addrWidth: Int = 32, val dataWidth: Int = 32) extends Bundle {
  val enable = Input(Bool())
  val addr = Input(UInt(addrWidth.W))
  val data = Output(UInt(dataWidth.W))
}

class WritePort (val addrWidth: Int = 32, val dataWidth: Int = 32) extends Bundle {
  val enable = Input(Bool())
  val addr = Input(UInt(addrWidth.W))
	val mask = Input(UInt((dataWidth / 8).W))
  val data = Input(UInt(dataWidth.W))
}
