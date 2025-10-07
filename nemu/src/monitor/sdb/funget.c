#include <stdio.h>
#include <stdlib.h>
#include <elf.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

void *parse_string_table(const void *addr);

void parse_symbol_table(const void *addr);

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
	
	if(ehdr->e_machine != EM_RISCV || ehdr->e_ident[EI_CLASS] != ELFCLASS32) {
		printf("Not a RV32 ELF file\n");
		exit(1);
	}
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
		if(ELF32_ST_TYPE(sym->st_info) != STT_FUNC) continue;
		printf("Symbol %d at: %p\n", j, sym);
		printf("Name: %s\n", (char *)(strtab_base + sym->st_name));
		printf("Value: %x\n", sym->st_value);
		printf("Size: %x\n", sym->st_size);
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