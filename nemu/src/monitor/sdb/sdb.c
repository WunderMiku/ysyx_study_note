/***************************************************************************************
* Copyright (c) 2014-2024 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <isa.h>
#include <cpu/cpu.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <memory/vaddr.h>
#include "sdb.h"
#include <errno.h>
#include <stdint.h>
#include <string.h>

static int is_batch_mode = false;

void init_regex();
void init_wp_pool();

/* We use the `readline' library to provide more flexibility to read from stdin. */
static char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}


static int cmd_q(char *args) {
  nemu_state.state = NEMU_QUIT;
  return -1;
}

static int cmd_help(char *args);

static int cmd_si(char *args);

static int cmd_info(char *args);

static int cmd_x(char *args);

static struct {
  const char *name;
  const char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display information about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
  { "si", "Execute N instructions step by step, N default 1", cmd_si},
  { "info", "Print information, r: register status, w: watchpoint information", cmd_info},
  {"x", "Scan memory", cmd_x}

  /* TODO: Add more commands */

};

#define NR_CMD ARRLEN(cmd_table)

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

static int cmd_si(char *args) {
  char *arg = strtok(NULL, " ");
  char *endptr;
  uint64_t n = 1;

  if(arg != NULL) {
    /* Have args */
    errno = 0;
    n = strtoul(arg, &endptr, 10);
    if(errno == ERANGE) {
      printf("Numerical result out of range.\n");
      return 0;
    }

    if(arg == endptr) {
      printf("No digits were found.\n");
      return 0;
    }
    
    cpu_exec(n);
  }

  /* No args */
  else cpu_exec(1);

  return 0;
}

static int cmd_info(char *args) {
  char *arg = strtok(NULL, " ");
  if (arg == NULL) {
    printf("USAGE : info r /  info w \n");
  } else {
    if (strcmp(arg, "r") == 0) {
      isa_reg_display();
    } else 
    if (strcmp(arg, "w") == 0) {
      TODO();
    } else {
      printf("USAGE : info r /  info w \n");
    }
  }

  return 0;
}

static int cmd_x(char *args) {
  char *arg = strtok(NULL, " ");
  if(arg == NULL) {
    printf("USAGE : x N EXPR \n");
    return 0;
  }

  char *endptr;
  int N = 0;
  errno = 0;
  N = strtoul(arg, &endptr, 10);
  if(errno == ERANGE) {
    printf("Numerical result out of range (N).\n");
    return 0;
  }

  if(arg == endptr) {
    printf("No digits were found (N).\n");
    return 0;
  }

  if(!(N == 1 || N == 2 || N == 4 || (ISDEF(CONFIG_ISA64) && N == 8))) {
    printf("The value of N is invalid.\n");
    return 0;
  }

  /* N is valid */

  char *addr_char = strtok(NULL, " ");
  if(addr_char == NULL) {
    printf("USAGE : x N EXPR \n");
    return 0;
  }

  errno = 0;
  unsigned long long temp_addr = 0;
  temp_addr = strtoull(addr_char, &endptr, 16);

  if(errno == ERANGE) {
    printf("Numerical result out of range (addr).\n");
    return 0;
  }

  if(addr_char == endptr) {
    printf("No digits were found (addr).\n");
    return 0;
  }

  if (temp_addr > UINT32_MAX && !ISDEF(CONFIG_ISA64)) {
    printf("Address out of range for 32-bit architecture.\n");
    return 0;
  }
  vaddr_t addr = (vaddr_t)temp_addr;

  /* Check if address is within physical memory bounds */
  if (addr < CONFIG_MBASE || addr > CONFIG_MBASE + CONFIG_MSIZE - 1) {
    printf("Address " FMT_WORD " is out of bounds. Valid range: [" FMT_WORD ", " FMT_WORD "]\n", 
           addr, CONFIG_MBASE, CONFIG_MBASE + CONFIG_MSIZE - 1);
    return 0;
  }

  /* addr is valid */

  printf("" FMT_WORD " at " FMT_WORD "\n", vaddr_read(addr, N), addr);
  
  return 0;
}

void sdb_set_batch_mode() {
  is_batch_mode = true;
}

void sdb_mainloop() {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL; ) {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef CONFIG_DEVICE
    extern void sdl_clear_event_queue();
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}

void init_sdb() {
  /* Compile the regular expressions. */
  init_regex();

  /* Initialize the watchpoint pool. */
  init_wp_pool();
}
