#ifndef __FUNGET_H__
#define __FUNGET_H__

#include <elf.h>
#include <stdbool.h>
#include "common.h"

#define FUNCT_NAME_MAX_LENGTH 128
#define FUCT_MAXSIZE 1024

typedef struct {
    char name[FUNCT_NAME_MAX_LENGTH];
    Elf32_Addr addr;
		Elf32_Word size;
} Func;

typedef struct {
	Func funcs[FUCT_MAXSIZE];
	uint32_t func_num;
	bool is_call, is_ret, is_move;
	int call_level;
} Funcget;

extern bool Elf_Files_Get;

void *parse_string_table(const void *addr);

void parse_symbol_table(const void *addr);

void get_function(char *file);

void init_funget();

void funget_detect(vaddr_t before_pc, vaddr_t pc, uint32_t inst);

#endif