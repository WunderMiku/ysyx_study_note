object Elaborate extends App {
  println("Elaborating module to SystemVerilog...")
  val firtoolOptions = Array(
    "--lowering-options=" + List(
      // make yosys happy
      // see https://github.com/llvm/circt/blob/main/docs/VerilogGeneration.md
      "disallowLocalVariables",
      "disallowPackedArrays",
      "locationInfoStyle=none"
    ).reduce(_ + "," + _)
  )
  circt.stage.ChiselStage.emitSystemVerilogFile(new idu.InstDecodeUnit(), args, firtoolOptions)
}
