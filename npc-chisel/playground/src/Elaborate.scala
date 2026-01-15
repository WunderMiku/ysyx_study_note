object Elaborate extends App {
  println("Elaborating module to SystemVerilog...")
  val firtoolOptions = Array(
    "-o=./build",
    "--split-verilog",
    "--lowering-options=" + List(
      // make yosys happy
      // see https://github.com/llvm/circt/blob/main/docs/VerilogGeneration.md
      "disallowLocalVariables",
      "disallowPackedArrays",
      "locationInfoStyle=none"
    ).reduce(_ + "," + _)
  )
  circt.stage.ChiselStage.emitSystemVerilogFile(new top.Top(), args, firtoolOptions)
}
