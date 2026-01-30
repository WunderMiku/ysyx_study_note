#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

/* watchpoint part */
#include <stdbool.h>
#include <stdint.h>
#define NR_WP 32
#define MAXSIZE 1024

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;

  bool breakpoint; // 是否是断点
  char * str;
  uint32_t old_result;

} WP;

void check_all_using_wp();
void init_wp_pool();
void list_all_using_wp();
void delete_all_using_wp();
WP *new_wp(char *expression, uint32_t initial_value, bool *success, bool breakpoint) ;
void free_wp(int, bool*);

#endif