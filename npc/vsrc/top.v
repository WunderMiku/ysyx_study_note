module top (
  input clk,
  input rst,
  output [7:0] HEX0,
  output [7:0] HEX1
);
  wire [3:0] pc_Q;
  wire pc_WE;
  wire [3:0] pc_D;

  wire jump = (reg_Q_R1 != reg_Q_R2);
  
  assign pc_D = addr_bner0; 
  assign pc_WE = bner0_opcode && jump;
  
  PC nPC (
    .clk(clk),
    .rst(rst),
    .D(pc_D),
    .WE(pc_WE),
    .Q(pc_Q)
  );

  wire [7:0] inst;
  wire [1:0] opcode, rd, rs1, rs2;
  wire [3:0] addr_bner0 = inst[5:2];
  wire [3:0] imm = inst[3:0];
  assign {opcode, rd, rs1, rs2} = inst;


  wire add_opcode = (opcode == 2'b00);
  wire out_opcode = (opcode == 2'b01);
  wire li_opcode = (opcode == 2'b10);
  wire bner0_opcode = (opcode == 2'b11);

  My_ROM nROM (
    .addr(pc_Q),
    .Q(inst)
  );


  wire [1:0] reg_addr;
  wire [1:0] reg_addr_R1, reg_addr_R2;
  wire reg_WE;
  wire [7:0] reg_D;
  wire [7:0] reg_Q;
  wire [7:0] reg_Q_R1, reg_Q_R2;

  wire [7:0] sum = reg_Q_R1 + reg_Q_R2;

  assign reg_addr = rd;
  assign reg_addr_R1 = rs1 & {2{add_opcode}};
  assign reg_addr_R2 = rs2;
  assign reg_WE = add_opcode | li_opcode;
  assign reg_D = {8{add_opcode}} & sum |
                 {8{li_opcode}} & {4'b0, imm};

  GPR nGPR (
    .clk(clk),
    .rst(rst),
    .addr(reg_addr),
    .addr_R1(rs1),
    .addr_R2(rs2),
    .WE(reg_WE),
    .D(reg_D),
    .Q(reg_Q),
    .Q_R1(reg_Q_R1),
    .Q_R2(reg_Q_R2)
  );

  seg_driver seg0 (
    .num(reg_Q[3:0]),
    .EN(out_opcode),
    .clk(clk),
    .rst(rst),
    .seg(HEX0)
  );

  seg_driver seg1 (
    .num(reg_Q[7:4]),
    .EN(out_opcode),
    .clk(clk),
    .rst(rst),
    .seg(HEX1)
  );

endmodule


module PC (
  input clk,
  input rst,
  input [3:0] D,
  input WE,
  output reg [3:0] Q
);

  always @(posedge clk or posedge rst) begin
    if (rst) begin
      Q <= 4'b0;
    end else if (WE) begin
      Q <= D;
    end else begin
      Q <= Q + 1;
    end
  end

endmodule


module GPR (
  input clk,
  input rst,
  input [1:0] addr,
  input [1:0] addr_R1,
  input [1:0] addr_R2,
  input WE,
  input [7:0] D,
  output [7:0] Q,
  output [7:0] Q_R1,
  output [7:0] Q_R2
);
  reg [7:0] regs [3:0];

  always @(posedge clk or posedge rst) begin
    if (rst) begin
      regs[0] <= 8'b0;
      regs[1] <= 8'b0;
      regs[2] <= 8'b0;
      regs[3] <= 8'b0;
    end else if (WE) begin
      regs[addr] <= D;
    end
  end

  assign Q = regs[addr];
  assign Q_R1 = regs[addr_R1];
  assign Q_R2 = regs[addr_R2];

endmodule


module My_ROM (
  input [3:0] addr,
  output [7:0] Q
);
  reg [7:0] roms [15:0];

  initial begin
    $readmemb("resource/inst.txt", roms);
  end

  assign Q = roms[addr];

endmodule


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