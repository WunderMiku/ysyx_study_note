module IDU (
	input [31:0] inst,
	output [4:0] rs1_addr,
	output [4:0] rs2_addr,
	output [4:0] rd_addr,
	output add_en,
	output addi_en,
	output lui_en,
	output lw_en,
	output lbu_en,
	output sw_en,
	output sb_en,
	output jalr_en,
	output ebreak_en,
	output [11:0] I_imm,
	output [11:0] S_imm,
	output [12:0] B_imm,
	output [31:0] U_imm,
	output [20:0] J_imm
);

wire [6:0] opcode = inst[6:0];
wire [2:0] funct3 = inst[14:12];
wire [6:0] funct7 = inst[31:25];

assign rd_addr = inst[11:7];
assign rs1_addr = inst[19:15];
assign rs2_addr = inst[24:20];

assign I_imm = inst[31:20];
assign S_imm = {inst[31:25], inst[11:7]};
assign B_imm = {inst[31], inst[7], inst[30:25], inst[11:8], 1'b0};
assign U_imm = {inst[31:12], 12'b0};
assign J_imm = {inst[31], inst[19:12], inst[20], inst[30:21], 1'b0};

assign add_en  =  (opcode == 7'b0110011) && (funct3 == 3'b000) && (funct7 == 7'b0000000);
assign addi_en =  (opcode == 7'b0010011) && (funct3 == 3'b000);
assign lui_en  =  (opcode == 7'b0110111);
assign lw_en   =  (opcode == 7'b0000011) && (funct3 == 3'b010);
assign lbu_en  =  (opcode == 7'b0000011) && (funct3 == 3'b100);
assign sw_en   =  (opcode == 7'b0100011) && (funct3 == 3'b010);
assign sb_en   =  (opcode == 7'b0100011) && (funct3 == 3'b000);
assign jalr_en =  (opcode == 7'b1100111) && (funct3 == 3'b000);
assign ebreak_en = (inst == 32'h00100073);

endmodule