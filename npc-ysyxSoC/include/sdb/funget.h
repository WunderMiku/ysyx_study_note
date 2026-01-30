#ifndef __FUNGET_H__
#define __FUNGET_H__

#include <cstdint>
#include <elf.h>
#include <stdbool.h>

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

uint8_t *parse_string_table(const void *addr);

void parse_symbol_table(const uint8_t *addr);

void get_function(char *file);

void init_funget();

void funget_detect(uint32_t before_pc, uint32_t pc, uint32_t inst);

#endif