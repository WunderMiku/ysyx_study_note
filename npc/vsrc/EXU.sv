module EXU (
	// 指令使能信号（form IDU)
	input add_en,
	input addi_en,
	input lui_en,
	input lw_en,
	input lbu_en,
	input sw_en,
	input sb_en,
	input jalr_en,
	input ebreak_en,
	input auipc_en,
	input jal_en,
	input sub_en,
	input sltiu_en,
	input beq_en,
	input bne_en,
	input sltu_en,
	input xor_en,
	input or_en,
	input sh_en,
	input srai_en,
	input andi_en,
	input sll_en,
	input and_en,

	// 数据输入
	input  [31:0] rs1_val,
	input  [31:0] rs2_val,
	input  [4:0]  rd_addr,
	input  [31:0] ram_read_val,
	input  [31:0] imm, // 均为符号扩展后的立即数

	// PC相关
	input  [31:0] pc,
	output [31:0] next_pc,

	// 内存接口
	// 写端口
	output        ram_we,
	output [31:0] ram_write_addr,
	output [31:0] ram_write_data,
	output [3:0]  ram_write_mask,
	

	// 读端口
	output        ram_re,
	output [31:0] ram_read_addr,

	// 寄存器接口
	output        reg_we,
	output [4:0]  reg_addr,
	output [31:0] reg_data
);
	// =========== 寄存器信号控制 =========== 
	// 寄存器写使能
	assign reg_we = (add_en | addi_en | jalr_en | lui_en | lbu_en | lw_en |
									 auipc_en | jal_en | sub_en | sltiu_en | sltu_en | xor_en |
									 or_en | srai_en | andi_en | sll_en | and_en) && (reg_addr != 5'b0);

	// 寄存器写入数据
	assign reg_data = ({32{add_en}} & (rs1_val + rs2_val)) |
	                  ({32{addi_en}} & (rs1_val + imm))    |
						        ({32{jalr_en}} & (pc + 4))           |
										({32{lui_en}} & {imm[31:12], 12'b0}) |
										({32{lbu_en}} & {24'b0, ram_read_offset_val}) |
										({32{lw_en}} & ram_read_val) |
										({32{auipc_en}} & (pc + imm)) |
										({32{jal_en}} & (pc + 4)) |
										({32{sub_en}} & (rs1_val - rs2_val)) |
										({32{sltiu_en}} & {31'b0, (rs1_val < imm)}) |
										({32{sltu_en}} & {31'b0, (rs1_val < rs2_val)}) |
										({32{xor_en}} & (rs1_val ^ rs2_val)) |
										({32{or_en}} & (rs1_val | rs2_val)) |
										({32{srai_en}} & (rs1_val >>> imm)) |
										({32{andi_en}} & (rs1_val & imm)) |
										({32{sll_en}} & (rs1_val << rs2_val[4:0])) |
										({32{and_en}} & (rs1_val & rs2_val));

	// 寄存器写入地址
	assign reg_addr = ({5{add_en | addi_en | jalr_en | lui_en | lbu_en | lw_en |
												auipc_en | jal_en | sub_en | sltiu_en | sltu_en | xor_en |
												or_en | srai_en | andi_en | sll_en | and_en}} & rd_addr) | 5'b0;


	// =========== PC信号控制 =========== 
	assign next_pc =  (jalr_en) ? ((rs1_val + imm) & 32'hFFFFFFFE) :
								  	(jal_en) ? (pc + imm) :
										(beq_en & (rs1_val == rs2_val)) ? (pc + imm) :
										(bne_en & (rs1_val != rs2_val)) ? (pc + imm) :
									  (pc + 4);


	// =========== 储存器信号控制 =========== 
	assign ram_we = sb_en | sw_en | sh_en;

	assign ram_write_data = ({32{sb_en}} & ({4{rs2_val[7:0]}})) |
	                        ({32{sw_en}} & (rs2_val[31:0])) |
													({32{sh_en}} & ({2{rs2_val[15:0]}}));

	assign ram_write_addr = ({32{sb_en | sw_en | sh_en}} & (rs1_val + imm));

	assign ram_write_mask = ({4{sb_en}} & {ram_write_addr[1:0] == 2'd3, 
																				 ram_write_addr[1:0] == 2'd2, 
																				 ram_write_addr[1:0] == 2'd1, 
																				 ram_write_addr[1:0] == 2'd0}) |
													({4{sw_en}})                                 |
													({4{sh_en}} & {ram_write_addr[1:0] == 2'd2, 
																				 ram_write_addr[1:0] == 2'd2, 
																				 ram_write_addr[1:0] == 2'd0, 
																				 ram_write_addr[1:0] == 2'd0});

	assign ram_re = lbu_en | lw_en;

	assign ram_read_addr = ({32{lbu_en | lw_en}} & (rs1_val + imm));


	// 单字节读取
	reg [7:0] ram_read_offset_val;
	always @(*) begin 
		case(ram_read_addr[1:0])
			2'd0: ram_read_offset_val = ram_read_val[7:0];
			2'd1: ram_read_offset_val = ram_read_val[15:8];
			2'd2: ram_read_offset_val = ram_read_val[23:16];
			2'd3: ram_read_offset_val = ram_read_val[31:24];
		endcase
	end


  // =========== DPI-C 信号传递 ===========
	export "DPI-C" function ebreak_get;
	function bit ebreak_get(output bit ebreak);
		ebreak = ebreak_en;
	endfunction

endmodule