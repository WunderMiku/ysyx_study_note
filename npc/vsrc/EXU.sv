module ysyx_25090244_EXU (
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
	input xori_en,
	input bge_en,
	input blt_en,
	input srli_en,
	input bgeu_en,
	input slli_en,
	input bltu_en,
	input sra_en,
	input srl_en,
	input lh_en,
	input lhu_en,
	input lb_en,
	input ori_en,
	input slti_en,
	input slt_en,
	input csrrc_en,
	input csrrs_en,
	input csrrw_en,
	input ecall_en,
	input mret_en,

	// 数据输入
	input  [31:0] rs1_val,
	input  [31:0] rs2_val,
	input  [4:0]  rd_addr,
	input  [31:0] ram_read_val,
	input  [31:0] imm, // 符号扩展后的立即数 (除有特殊要求的个别指令)

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
	output [31:0] reg_data,

	// CSR 读接口
	output [11:0] csr_raddr,
	input  [31:0] csr_rdata,

	// CSR 写接口
	output csr_we,
	output [11:0] csr_waddr,
	output [31:0] csr_wdata,

	// CSR 附加写接口
	output csr_we1,
	output [11:0] csr_waddr1,
	output [31:0] csr_wdata1
);
	// =========== 寄存器信号控制 =========== 
	// 寄存器写使能
	assign reg_we = (add_en | addi_en | jalr_en | lui_en | lbu_en | lw_en |
									 auipc_en | jal_en | sub_en | sltiu_en | sltu_en | xor_en |
									 or_en | srai_en | andi_en | sll_en | and_en | xori_en |
									 srli_en | slli_en | sra_en | srl_en | lh_en | lhu_en | 
									 lb_en | ori_en | slti_en | slt_en | csrrc_en | csrrs_en|
									 csrrw_en) && (reg_addr != 5'b0);

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
										({32{srai_en}} & (rs1_val[31] ? (32'hFFFFFFFF << (32 - imm) | (rs1_val >> imm)) : 
										(rs1_val >> imm))) | // 不支持算术右移运算符

										({32{andi_en}} & (rs1_val & imm)) |
										({32{sll_en}} & (rs1_val << rs2_val[4:0])) |
										({32{and_en}} & (rs1_val & rs2_val)) |
										({32{xori_en}} & (rs1_val ^ imm)) |
										({32{srli_en}} & (rs1_val >> imm)) |
										({32{slli_en}} & (rs1_val << imm)) |
										({32{sra_en}} & (rs1_val[31] ? (32'hFFFFFFFF << (32 - rs2_val[4:0]) | (rs1_val >> rs2_val[4:0])) : 
										(rs1_val >> rs2_val[4:0]))) | // 不支持算术右移运算符

										({32{srl_en}} & (rs1_val >> rs2_val[4:0])) |
										({32{lh_en}} & ({{16{ram_read_offset_hval[15]}}, ram_read_offset_hval})) |
										({32{lhu_en}} & ({16'b0, ram_read_offset_hval})) |
										({32{lb_en}} & ({{24{ram_read_offset_val[7]}}, ram_read_offset_val})) |
										({32{ori_en}} & (rs1_val | imm)) |
										({32{slti_en}} & {31'b0, ($signed(rs1_val) < $signed(imm))}) |
										({32{slt_en}} & {31'b0, ($signed(rs1_val) < $signed(rs2_val))}) |
										({32{csrrc_en | csrrs_en | csrrw_en}} & (csr_rdata));

	// 寄存器写入地址
	assign reg_addr = ({5{add_en | addi_en | jalr_en | lui_en | lbu_en | lw_en |
												auipc_en | jal_en | sub_en | sltiu_en | sltu_en | xor_en |
												or_en | srai_en | andi_en | sll_en | and_en | xori_en |
												srli_en | slli_en | sra_en | srl_en | lh_en | lhu_en |
												lb_en | ori_en | slti_en | slt_en | csrrc_en | csrrs_en|
												csrrw_en}} & rd_addr) | 5'b0;
												

	// =========== PC信号控制 =========== 
	assign next_pc =  (jalr_en) ? ((rs1_val + imm) & 32'hFFFFFFFE) :
								  	(jal_en) ? (pc + imm) :
										(beq_en & (rs1_val == rs2_val)) ? (pc + imm) :
										(bne_en & (rs1_val != rs2_val)) ? (pc + imm) :
										(bge_en & ($signed(rs1_val) >= $signed(rs2_val))) ? (pc + imm) : // 支持有符号比较
										(blt_en & ($signed(rs1_val) < $signed(rs2_val))) ? (pc + imm) :
										(bgeu_en & (rs1_val >= rs2_val)) ? (pc + imm) :
										(bltu_en & (rs1_val < rs2_val)) ? (pc + imm) :
									  (ecall_en) ? (csr_rdata) :
										(mret_en) ? (csr_rdata) :
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

	assign ram_re = lbu_en | lw_en | lh_en | lhu_en | lb_en;

	assign ram_read_addr = ({32{lbu_en | lw_en | lh_en | lhu_en | lb_en}} & (rs1_val + imm));


	// 单字节读取
	reg [7:0] ram_read_offset_val;
	reg [15:0] ram_read_offset_hval;
	always @(*) begin 
		if(lbu_en | lb_en) begin
			case(ram_read_addr[1:0])
				2'd0: ram_read_offset_val = ram_read_val[7:0];
				2'd1: ram_read_offset_val = ram_read_val[15:8];
				2'd2: ram_read_offset_val = ram_read_val[23:16];
				2'd3: ram_read_offset_val = ram_read_val[31:24];
			endcase
		end

		if(lh_en | lhu_en) begin
			case(ram_read_addr[1:0])
				2'd0: ram_read_offset_hval = ram_read_val[15:0];
				2'd2: ram_read_offset_hval = ram_read_val[31:16];
				default: ram_read_offset_hval = 16'b0;
			endcase
		end
	end


	// =========== CSR信号控制 =========== 
	assign csr_we = (csrrc_en | csrrs_en | csrrw_en | ecall_en);

	assign csr_waddr = ({12{csrrc_en | csrrs_en | csrrw_en}} & imm[11:0]) |
		                 ({12{ecall_en}} & 12'h341) | // mepc
	                   12'b0;

	assign csr_wdata = ({32{csrrc_en}} & ((~rs1_val) | csr_rdata)) |
			               ({32{csrrs_en}} & (rs1_val | csr_rdata)) |
										 ({32{csrrw_en}} & rs1_val) |
										 ({32{ecall_en}} & (pc)) |
	                   32'b0; 
	
	assign csr_raddr = ({12{csrrc_en | csrrs_en | csrrw_en}} & imm[11:0]) |
		                 ({12{ecall_en}} & 12'h305) | // mtvec
										 ({12{mret_en}} & 12'h341) |  // mepc
	                   12'b0;
	
	assign csr_we1 = ecall_en;

	assign csr_waddr1 = ({12{ecall_en}} & 12'h342) | // mcause
	                    12'b0; 

	assign csr_wdata1 = ({32{ecall_en}} & (32'h0000000b)) |
	                    32'b0;

  // =========== DPI-C 信号传递 ===========
	export "DPI-C" function ebreak_get;
	function bit ebreak_get(output bit ebreak);
		ebreak = ebreak_en;
	endfunction

endmodule