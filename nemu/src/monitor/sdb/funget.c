#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <elf.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "funget.h"
#include "debug.h"
#include "utils.h"


Funcget fun_get;
bool Elf_Files_Get = true;
void funget_detect(vaddr_t before_pc, vaddr_t pc, uint32_t inst) {

	fun_get.is_call = false;
	fun_get.is_ret = false;
	fun_get.is_move = false;

	// 检测指令部分
	uint32_t inst_opcode = inst & 0x7f;
	uint32_t inst_rd     = (inst >> 7) & 0x1f;
	uint32_t inst_rs1    = (inst >> 15) & 0x1f;
	uint32_t inst_offset = (inst >> 20);

	int before_fun_index = -1;
	int fun_index = -1;

	for(int i = 0; i < fun_get.func_num; i++) {
			Elf32_Addr fun_addr = fun_get.funcs[i].addr;
			Elf32_Addr fun_endaddr = fun_get.funcs[i].addr + fun_get.funcs[i].size;

			if(before_pc >= fun_addr && before_pc < fun_endaddr) before_fun_index = i;
			if(pc >= fun_addr && pc < fun_endaddr) fun_index = i;
	}

	if(inst_opcode == 0x6F) { 
		if(inst_rd != 0x0) { 	// jal (保存返回值)
			fun_get.is_call = true;
		} else if(before_fun_index != fun_index) {
			fun_get.is_move = true;
		}
	}

	if(inst_opcode == 0x67) { 
		if(inst_rd != 0x0) {	 // jalr (保存返回值）
			fun_get.is_call = true;
		} else if(before_fun_index != fun_index) {
			fun_get.is_move = true;
		}
	}

	if(inst_opcode == 0x67 && inst_rd == 0x0 && inst_rs1 == 0x1 && inst_offset == 0) { // ret
		fun_get.is_ret = true;
	}

	if(!(fun_get.is_call || fun_get.is_ret || fun_get.is_move)) return; // 都不是，不输出

	// 输出部分
	if (!(before_fun_index >= 0 && fun_index >= 0)) {
		if (before_fun_index >= 0){
			printf(ANSI_FG_RED " (MISS) " ANSI_NONE ANSI_FG_BLACK "%s(0x%08x) -> (0x%08x)\n",fun_get.funcs[before_fun_index].name, before_pc, pc);
		} else if (fun_index >= 0) {
			printf(ANSI_FG_RED " (MISS) " ANSI_NONE ANSI_FG_BLACK "(0x%08x)-> %s(0x%08x)\n", before_pc,  fun_get.funcs[fun_index].name, pc);
		} else {
			printf(ANSI_FG_RED " (MISS) " ANSI_NONE ANSI_FG_BLACK "(0x%08x) -> (0x%08x)\n", before_pc, pc);
		}
		return;
	}
	assert(before_fun_index != fun_index);

	if(fun_get.is_call) {
		fun_get.call_level++;
		int i = fun_get.call_level - 1;
		while(i-- > 0) printf(" ");
		printf(ANSI_FG_GREEN " (Call) " ANSI_NONE ANSI_FG_YELLOW "%s" ANSI_NONE "(0x%08x) -> " ANSI_FG_GREEN "%s"\
			 ANSI_NONE "(0x%08x)\n",fun_get.funcs[before_fun_index].name, before_pc, fun_get.funcs[fun_index].name, pc);
	}

	if(fun_get.is_ret) {
		fun_get.call_level--;
		int i = fun_get.call_level - 1;
		while(i-- > 0) printf(" ");
		printf(ANSI_FG_YELLOW " (Ret) " ANSI_NONE ANSI_FG_GREEN "%s"ANSI_NONE"(0x%08x) <- " ANSI_FG_YELLOW "%s"\
			 ANSI_NONE "(0x%08x)\n",fun_get.funcs[fun_index].name, pc, fun_get.funcs[before_fun_index].name, before_pc);
	}

	if(fun_get.is_move && !fun_get.is_ret) {
		int i = fun_get.call_level - 1;
		while(i-- > 0) printf(" ");
		printf(ANSI_FG_CYAN " (Jmp) " ANSI_NONE ANSI_FG_BLACK "%s -> %s \n"\
			,fun_get.funcs[before_fun_index].name, fun_get.funcs[fun_index].name);
	}
}

void init_funget() {
	memset(&fun_get, 0, sizeof(fun_get));
}


void funget_set_function(char *name, Elf32_Addr addr, Elf32_Word size) {
	assert(fun_get.func_num < FUCT_MAXSIZE);
	assert(name != NULL);
	strncpy(fun_get.funcs[fun_get.func_num].name, name, FUNCT_NAME_MAX_LENGTH);
	fun_get.funcs[fun_get.func_num].addr = addr;
	fun_get.funcs[fun_get.func_num].size = size;
	fun_get.func_num++;
	// Log("%d | Function %s at: 0x%x, size: %x\n", fun_get.func_num - 1, fun_get.funcs[fun_get.func_num - 1].name, fun_get.funcs[fun_get.func_num - 1].addr, fun_get.funcs[fun_get.func_num - 1].size);
}


/*
 * 从指定的ELF文件中获取并解析函数符号信息
 * @param file: ELF文件路径指针
 */
void get_function(char *file)
{
	if(file == NULL) {
    printf("NO files\n");
		exit(1);
	}
	int fd = -1;
	void *map_addr = NULL;

	struct stat st;

	fd = open(file, O_RDONLY); // 只读打开
	if(fd == -1) {
		printf("open %s failed\n", file);
		exit(1);
	}

	int rt = fstat(fd, &st);
	if(rt == -1) {
		printf("fstat %s failed\n", file);
		close(fd);
		exit(1);
	}

	// 获取文件大小
	size_t file_size = st.st_size;

	// 映射文件
	map_addr = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fd, 0);

	if(map_addr == MAP_FAILED) {
		printf("mmap %s failed\n", file);
		close(fd);
		exit(1);
	}

	// map_addr 指向文件开头，也就是 ELF 头
	const Elf32_Ehdr *ehdr = (const Elf32_Ehdr *)map_addr;
	
	// 验证是否为RV32 ELF文件
	if(ehdr->e_ident[0] != 0x7f || ehdr->e_ident[1] != 'E' || \
		 ehdr->e_ident[2] != 'L'  || ehdr->e_ident[3] != 'F') {
			printf("Not a ELF file\n");
			close(fd);
			exit(1);
		 }
	
	if(ehdr->e_machine != EM_RISCV || ehdr->e_ident[EI_CLASS] != ELFCLASS32) {
		printf("Not a RV32 ELF file\n");
		close(fd);
		exit(1);
	}

	// 解析符号表信息
	parse_symbol_table(map_addr);
}


void *parse_string_table(const void *addr) {
	const Elf32_Ehdr *ehdr = (const Elf32_Ehdr *)addr;
	Elf32_Off shoff = ehdr->e_shoff;
	if(shoff == 0) {
		printf("No section header table\n");
		exit(1);
	}

	// 获取节头表基地址
	void *shdr_base = (void *)(addr + shoff);
	// printf("Section header table at: %p\n", shdr_base);

	// 获取节头表表项大小
	Elf32_Half shentsize = ehdr->e_shentsize;

	// 获取字符串表地址与内容字符串地址
	Elf32_Half shstrndx = ehdr->e_shstrndx;
	if(shstrndx == SHN_UNDEF) {
		printf("No string table\n");
		exit(1);
	}

	const Elf32_Shdr *shstrhdr = ( const Elf32_Shdr *)(shdr_base + shstrndx * shentsize);
	void *shstr_base = (void *)(addr + shstrhdr->sh_offset);

	return shstr_base;
}

void parse_symbol_table(const void *addr) {
	const Elf32_Ehdr *ehdr = (const Elf32_Ehdr *)addr;

	Elf32_Off shoff = ehdr->e_shoff;
	if(shoff == 0) {
		printf("No section header table\n");
		exit(1);
	}

	// 获取节头表基地址
	void *shdr_base = (void *)(addr + shoff);
	// printf("Section header table at: %p\n", shdr_base);

	// 获取表项数量和表项大小
	Elf32_Half shnum = ehdr->e_shnum;
	Elf32_Half shentsize = ehdr->e_shentsize;

	// 获取字符串表，符号表，符号字符串表数据基指针
	void *shstr_base = parse_string_table(addr);
	void *sym_base = NULL;
	void *strtab_base = NULL;

	int sym_num = 0;

	for(int i = 0; i < shnum; i++) {
		const Elf32_Shdr *shdr = (const Elf32_Shdr *)(shdr_base + i * shentsize);
		const char *section_name = shstr_base + shdr->sh_name;
		// printf("Section header %d at: %p\n", i, shdr);
		if(strcmp(section_name, ".symtab") == 0) {
			sym_base = (void *)(addr + shdr->sh_offset);
			sym_num = shdr->sh_size / sizeof(Elf32_Sym);
		}
		if(strcmp(section_name, ".strtab") == 0) {
			strtab_base = (void *)(addr + shdr->sh_offset);
		}
	}

	if(sym_base == NULL || strtab_base == NULL) {
		printf("No symbol table\n");
		exit(1);
	}

	for(int j = 0; j < sym_num; j++) {
		const Elf32_Sym *sym = (const Elf32_Sym *)(sym_base + j * sizeof(Elf32_Sym));
		if(ELF32_ST_TYPE(sym->st_info) != STT_FUNC) continue; // 仅处理函数类型符号
		// printf("Symbol %d at: %p\n", j, sym);
		// printf("Name: %s\n", (char *)(strtab_base + sym->st_name));
		// printf("Value: %x\n", sym->st_value);
		// printf("Size: %x\n", sym->st_size);
		funget_set_function((char *)(strtab_base + sym->st_name), sym->st_value, sym->st_size);
	}	
}

// 输出elf节头表
void parse_section_header_table(const void *addr) {
	const Elf32_Ehdr *ehdr = (const Elf32_Ehdr *)addr;

	Elf32_Off shoff = ehdr->e_shoff;
	if(shoff == 0) {
		printf("No section header table\n");
		exit(1);
	}

	// 获取节头表基地址
	void *shdr_base = (void *)(addr + shoff);
	printf("Section header table at: %p\n", shdr_base);

	// 获取表项数量和表项大小
	Elf32_Half shnum = ehdr->e_shnum;
	Elf32_Half shentsize = ehdr->e_shentsize;

	// 获取字符串表的字符串指针
	void *shstr_base = parse_string_table(addr);

	for(int i = 0; i < shnum; i++) {
		const Elf32_Shdr *shdr = (const Elf32_Shdr *)(shdr_base + i * shentsize);
		const char *section_name = shstr_base + shdr->sh_name;
		printf("Section header %d at: %p\n", i, shdr);
		printf("Name: %s\n", section_name);
		printf("Type: %x\n", shdr->sh_type);
		printf("Flags: %x\n", shdr->sh_flags);
		printf("Size: %x\n", shdr->sh_size);
	}
}