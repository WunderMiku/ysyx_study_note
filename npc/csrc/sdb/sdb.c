#include <readline/history.h>
#include <readline/readline.h>
#include <stdlib.h>
#include "npc.h"
#include <macro.h>

// TODO : 实现sdb指令与MainLoop的实现
static char* inputGets() {
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
  return 0;
}


static int cmd_q(char *args) {
  return -1;
}

static int cmd_help(char *args);

static int cmd_si(char *args);

static int cmd_info(char *args);

static int cmd_x(char *args);

static int cmd_exp(char *args);

static int cmd_w(char* args);

static int cmd_d(char* args);

static int cmd_b(char* args);


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
  {"x", "Scan memory", cmd_x},
  {"exp", "tmp, just test exper", cmd_exp},
  {"w", "add watchpoint", cmd_w},
  {"d", "delete watchpoint", cmd_d},
  {"b", "set breakpoint", cmd_b}

  /* TODO: Add more commands */

};

#define NR_CMD ARRLEN(cmd_table)



void sdbMainLoop() { // TODO
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

static int cmd_si(char *args) {
	printf("Single step\n");
	return exec(1);
}