module seg_driver (
  input [3:0] num,
  input EN,
  input clk,
  input rst,
  output [7:0] seg
);
  reg [7:0] mux_seg, mux_seg_save;
	MuxKey #(16, 4, 8) i0 (mux_seg, num, {
	4'd0,  8'b0000_0010, // 0
	4'd1,  8'b1001_1111, // 1
	4'd2,  8'b0010_0101, // 2
	4'd3,  8'b0000_1101, // 3
	4'd4,  8'b1001_1001, // 4
	4'd5,  8'b0100_1001, // 5
	4'd6,  8'b0100_0001, // 6
	4'd7,  8'b0001_1111, // 7
	4'd8,  8'b0000_0001, // 8
	4'd9,  8'b0000_1001, // 9
	4'd10, 8'b0001_0001, // A
	4'd11, 8'b1100_0001, // b
	4'd12, 8'b0110_0010, // C
	4'd13, 8'b1000_0101, // d
	4'd14, 8'b0110_0001, // E
	4'd15, 8'b0111_0001  // F
});

  reg out_flag;
  always @(posedge clk or posedge rst) begin
    if(rst) begin
      out_flag <= 1'b0;
      mux_seg_save <= 8'b0;
    end
    else if(EN) begin
      out_flag <= 1'b1;
      mux_seg_save <= mux_seg;
    end
    else begin
      mux_seg_save <= mux_seg_save;
      out_flag <= out_flag;
    end
  end

  assign seg = out_flag? mux_seg_save : 8'b1111_1111;

endmodule