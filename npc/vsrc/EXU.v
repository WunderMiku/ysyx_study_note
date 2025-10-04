module EXU (
	// 控制信号
	input add_en,
	input addi_en,
	input lui_en,
	input lw_en,
	input lbu_en,
	input sw_en,
	input sb_en,
	input jalr_en,

	// 数据输入
	input  [31:0] rs1_val,
	input  [31:0] rs2_val,
	input  [4:0]  rd_addr,
	input  [31:0] imm, // 均为符号扩展后的立即数

	// PC相关
	input  [31:0] pc,
	output [31:0] next_pc,

	// 内存接口
	output        ram_we,
	output [31:0] ram_addr,
	output [31:0] ram_data,
	
	// 寄存器接口
	output        reg_we,
	output [4:0]  reg_addr,
	output [31:0] reg_data
);
	// =========== 寄存器信号控制 =========== 
	assign reg_we = (add_en | addi_en | jalr_en) && (reg_addr != 5'b0);

	assign reg_data = ({32{add_en}} & (rs1_val + rs2_val)) | ({32{addi_en}} & (rs1_val + imm)) |
						        ({32{jalr_en}} & (pc + 4));

	assign reg_addr = ({5{add_en | addi_en | jalr_en}} & rd_addr) | 5'b0;


	// =========== PC信号控制 =========== 
	assign next_pc = (jalr_en)? ((rs1_val + imm) & 32'hFFFFFFFE) : (pc + 4);


	// =========== 储存器信号控制 =========== 
	assign ram_we = 1'b0;

	assign ram_data = 32'b0;

	assign ram_addr = 32'b0;


endmodule