#include <readline/history.h>
#include <readline/readline.h>
#include <stdlib.h>
#include "npc.h"
#include "reg.h"
#include "ram.h"
#include "sdb.h"
#include <macro.h>

static char* inputGets() {
  static char *lineRead = NULL;

  if (lineRead) {
    free(lineRead);
    lineRead = NULL;
  }
  char nowPc[32];
  snprintf(nowPc, 32, "(npc " COLOR_YELLOW "@0x%08x" COLOR_NONE ") ", dut->out_pc);
  lineRead = readline(nowPc);

  if (lineRead && *lineRead) {
    add_history(lineRead);
  }

  return lineRead;
}

static int cmd_c(char *args) {
  cpuExec(-1);
  return 0;
}


static int cmd_q(char *args) {
  npcState.state = NPC_QUIT;
  return -1;
}

// static int cmd_help(char *args);

static int cmd_si(char *args);

static int cmd_info(char *args);

static int cmd_x(char *args);

static int cmd_exp(char *args);

// static int cmd_w(char* args);

// static int cmd_d(char* args);

// static int cmd_b(char* args);


static struct {
  const char *name;
  const char *description;
  int (*handler) (char *);
} cmd_table [] = {
  // { "help", "Display information about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
  { "si", "Execute N instructions step by step, N default 1", cmd_si},
  { "info", "Print information, r: register status, w: watchpoint information", cmd_info},
  {"x", "Scan memory", cmd_x},
  {"exp", "tmp, just test exper", cmd_exp},
  // {"w", "add watchpoint", cmd_w},
  // {"d", "delete watchpoint", cmd_d},
  // {"b", "set breakpoint", cmd_b}

  /* TODO: Add more commands */

};

#define NR_CMD ARRLEN(cmd_table)



void sdbMainLoop() { 
	for (char *str; (str = inputGets()) != NULL; ) {
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
    
    cpuExec(n);
  }

  /* No args */
  else cpuExec(1);

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
      // list_all_using_wp();
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

  if(!(N == 1 || N == 2 || N == 4)) {
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

  if (temp_addr > UINT32_MAX) {
    printf("Address out of range for 32-bit architecture.\n");
    return 0;
  }
  uint32_t addr = (uint32_t)temp_addr;

  /* Check if address is within physical memory bounds */
  if (addr < MEM_BASE || addr > MEM_BASE + MEM_SIZE - 1) {
    printf("Address 0x%08x is out of bounds. Valid range: [0x%08x, 0x%08x]\n", 
           addr, MEM_BASE, MEM_BASE + MEM_SIZE - 1);
    return 0;
  }

  /* addr is valid */

  printf("0x%08x at 0x%08x\n", paddr_read(addr, N), addr);
  
  return 0;
}

int cmd_exp (char *args) { //tmp command
  char *arg = strtok(args, " ");
  if (arg == NULL) {
    printf("Need args, such as: exp cal <exp>, exp test <filePath> \n");
    return 0;
  }

  if (strcmp(arg, "cal") == 0) {
    char *expression_cal = arg + strlen(arg) + 1;
    if(expression_cal == NULL) {
      printf("Need <exp>. \n");
      return 0;
    }

    bool success = true;
    uint32_t consult;

    consult = expr(expression_cal, &success);
    if(success) printf("consult: 0x%08x\n", consult);
    
    else printf("expr ERROR! \n");
    
    return 0;
  }

  if(strcmp(arg, "test") == 0) {
    char *path = strtok(NULL, " ");
    if(path == NULL) {
      printf("Need <exp>. \n");
      return 0;
    }

    FILE *fp;
    char line [65536];

    fp = fopen(path, "r");
    if(fp == NULL) {
      printf("File open failed. \n");
      return 0;
    }

    uint32_t success_count = 0, failed_count = 0;
    while(fgets(line, sizeof(line), fp) != NULL) {

      // debug
      // printf("Raw line: '%s'\n", line);
      // printf("Line as bytes: ");
      // for(int i = 0; i < strlen(line); i++) {
      //   printf("%d ", (unsigned char)line[i]);
      // }
      // printf("\n");
      int len = strlen(line);
      if(line[len - 1] == '\n') {
        line[len - 1] = '\0';
      }

      char *answer = strtok(line, " ");
      if(answer == NULL) {
        printf("Failed to get the answer. \n");
        return 0;
      }

      errno = 0;
      char *endptr;
      uint32_t answer_int = strtoul(answer, &endptr, 10);
      if(errno == ERANGE) {
        printf("answer result out of range. \n");
        return 0;
      }

      if(answer == endptr) {
        printf("No digits were found.\n");
        return 0;
      }

      /* answer_int is valid */

      char *expression = strtok(NULL, " ");
      if(expression == NULL) {
        printf("Failed to get the expression. \n");
        return 0;
      }

      bool success = true;
      uint32_t expression_exp = expr(expression, &success);
      if(!success) {
        printf("expr failed! \n");
        return 0;
      }

      if(expression_exp == answer_int) {
        success_count ++;
      } else {
        failed_count ++;
      }
    }
    printf("SUCCESS: %d; FAILED: %d \n", success_count, failed_count);
    if(!failed_count) {
      printf("ALL TEST PASS! \n");
    }
    return 0;
  }

  printf("Unknown command. \n");
  return 0;
}

// static int cmd_w(char* args) {
//   // 如果没有开启watchpoint功能
//   #ifndef CONFIG_WATCHPOINT
//   printf("Watchpoint feature is disabled. Please enable it in the menuconfig. \n");
//   return 0;
//   #endif

//   if(args == NULL) {
//     printf("Need an expression! \n");
//     return 0;
//   }
//   uint32_t result = 0;
//   bool success = true;
//   result = expr(args, &success);
//   if(!success) {
//     printf("Invalid expression! \n");
//     return 0;
//   }
//   if(strlen(args) > MAXSIZE) {
//     printf("expression too long! \n");
//     return 0;
//   }
//   success = true;
//   new_wp(args, result, &success, false);
//   if(success) {
//     printf("A new watchpoint has been established. \n");
//   } else {
//     printf("Failed to establish a new watchpoint. \n");
//   }
//   return 0;
// }

// static int cmd_d(char* args) {
//   char *arg = strtok(NULL, " ");
//   if(arg == NULL) {
//     printf("Failed to get the NO. \n");
//     return 0;
//   }

//   errno = 0;
//   char *endptr;
//   uint32_t NO = strtoul(arg, &endptr, 10);
//   if(errno == ERANGE) {
//     printf("No result out of range. \n");
//     return 0;
//   }

//   if(arg == endptr) {
//     printf("No digits were found. \n");
//     return 0;
//   }

//   if(NO >= NR_WP) {
//     printf("NO out of range. \n");
//     return 0;
//   }
//   bool success = true;
//   free_wp(NO, &success);
//   if(success) {
//     printf("Successfully free watchpoint (NO:%d). \n", NO);
//   } else {
//     printf("Failed to free watchpoint. \n");
//   }
//   return 0;
// }

// static int cmd_b(char* args) {
//   if(args == NULL) {
//     printf("Need an address! \n");
//     return 0;
//   }

//   errno = 0;
//   char *endptr;
//   word_t temp_addr = strtoul(args, &endptr, 16);

//   if(errno == ERANGE) {
//     printf("Numerical result out of range. \n");
//     return 0;
//   }

//   if(args == endptr) {
//     printf("No digits were found. \n");
//     return 0;
//   }

//   if (temp_addr > UINT32_MAX && !ISDEF(CONFIG_ISA64)) {
//     printf("Address out of range for 32-bit architecture. \n");
//     return 0;
//   }
//   uint32_t addr = (uint32_t)temp_addr;

//   /* Check if address is within physical memory bounds */
//   if (addr < MEM_BASE || addr > MEM_BASE + MEM_SIZE - 1) {
//     printf("Address 0x%08x is out of bounds. Valid range: [0x%08x, 0x%08x]\n", 
//            addr, MEM_BASE, MEM_BASE + MEM_SIZE - 1);
//     return 0;
//   }
//   char str[MAXSIZE];
//   snprintf(str, MAXSIZE, "$pc == 0x%08x", addr);

//   bool success = true;
//   uint32_t result = expr(str, &success);
//   if(!success) {
//     printf("Invalid expression! \n");
//     return 0;
//   }

//   new_wp(str, result, &success, true);

//   if(success) {
//     printf("A new breakpoint has been established at address 0x%08x. \n", addr);
//   } else {
//     printf("Failed to establish a new breakpoint. \n");
//   }
  
//   return 0;
// }