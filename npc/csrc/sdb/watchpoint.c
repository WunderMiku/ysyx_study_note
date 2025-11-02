/***************************************************************************************
 * Copyright (c) 2014-2024 Zihao Yu, Nanjing University
 *
 * NEMU is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan
 *PSL v2. You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY
 *KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
 *NON-INFRINGEMENT, MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 *
 * See the Mulan PSL v2 for more details.
 ***************************************************************************************/

#include "watchpoint.h"
#include "npc.h"
#include "sdb.h"
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
  }

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */

WP *new_wp(char *expression, uint32_t initial_value, bool *success, bool breakpoint) {
  if(expression == NULL) {
    printf("null expression!\n");
    *success = false;
    return NULL;
  }

  if (free_ == NULL) {
    printf("NO free watchpoint!\n");
    assert(0);
  }

  // 分配一个新的watchpoint的链表操作
  WP *new_wp = free_;
  free_ = free_->next;

  WP *first_busy_wp = head;
  head = new_wp;
  new_wp->next = first_busy_wp;

  // 设置watchpoint的其他属性

  // 分配表达式的内存空间
  new_wp->str = (char *)calloc(sizeof(char), MAXSIZE + 1);
  assert(new_wp->str);

  // 复制表达式
  strncpy(new_wp->str, expression, MAXSIZE);
  new_wp->str[MAXSIZE] = '\0'; // 保证字符串合法

  new_wp->breakpoint = breakpoint;
  new_wp->old_result = initial_value;

  if(breakpoint) {
    printf("breakpoint(NO:%d; expression: %s) \n", new_wp->NO, new_wp->str);
  } else {
    printf("watchpoint(NO:%d; expression: %s) \n", new_wp->NO, new_wp->str);
  }

  if(breakpoint) {
    printf("Successfully returned a free breakpoint\n");
  } else {
    printf("Successfully returned a free watchpoint\n");
  }
  
  return new_wp;
}

void free_wp(int wp_no, bool *success) {
  WP *wp = NULL;
  for(int i = 0; i < NR_WP; i++) {
    if(wp_pool[i].NO == wp_no) {
      wp = &wp_pool[i];
      break;
    }
  }

  if (wp == NULL) {
    printf("Try to free a null watchpoint!\n");
    *success = false;
    return;
  }

  bool find = false;
  if (head == wp) {
    // 如果头节点就是wp
    head = head->next;
    find = true;
  } else {
    // 遍历head链表，寻找wp的前一个节点
    for (WP *wp_ptr = head; wp_ptr != NULL; wp_ptr = wp_ptr->next) {
      if (wp_ptr->next == wp) {
        wp_ptr->next = wp->next;
        find = true;
        break;
      }
    }
  }

  // 如果都没找到
  if (!find) {
    printf("Try to release a free or invalid watchpoint!\n");
    *success = false;
    return;
  }

  if(wp->str != NULL) {
    free(wp->str);
  }

  wp->old_result = 0;

  wp->next = free_;
  free_ = wp;
  *success = true;
  printf("Successfully released watchpoint\n");
}

void check_all_using_wp() {
  for (WP *wp_ptr = head; wp_ptr != NULL; wp_ptr = wp_ptr->next) {
      char *str = wp_ptr->str;
      uint32_t old_result = wp_ptr->old_result;
      bool breakpoint = wp_ptr->breakpoint;
      int NO = wp_ptr->NO;

      if(str == NULL) {
        printf("Have a NULL using_watchpoint NO:%d\n", NO);
        continue;
      }
      bool success = true;
      uint32_t result = expr(str, &success);
      if(!success) {
        if(!breakpoint) 
          printf("watchpoint(NO:%d): Expression parsing failed\n", NO);
        else
          printf("breakpoint(NO:%d): Expression parsing failed\n", NO);
        
        continue;
      }
      if((result != old_result) && !breakpoint) { // 当值改变且是watchpoint时触发
        if(npcState.state == NPC_RUNNING) { // 因为程序退出与暂停均依靠这个判断，所以目前只能这样
          npcState.state = NPC_STOP;
          printf("watchpoint %d triggered (expression : %s). \n", NO, str);
          printf("Old value = 0x%08x\nNew value = 0x%08x\n", old_result, result);
          wp_ptr->old_result = result; // 更新值
        } else {
          printf("watchpoint %d triggered but nemu has already stopped or ended (expression : %s). \n", NO, str);
        }
      }
      if(result && breakpoint) { // 当值为1且是breakpoint时触发
        if(npcState.state == NPC_RUNNING) { // 因为程序退出与暂停均依靠这个判断，所以目前只能这样
          npcState.state = NPC_STOP;
          printf("breakpoint %d triggered (expression : %s). \n", NO, str);
          wp_ptr->old_result = result; // 更新值
        } else {
          printf("breakpoint %d triggered but nemu has already stopped or ended (expression : %s). \n", NO, str);
        }
      }
  }
}

void list_all_using_wp() {
  printf("No  value   What\n");
  for (WP *wp_ptr = head; wp_ptr != NULL; wp_ptr = wp_ptr->next) {
      char *str = wp_ptr->str;
      int NO = wp_ptr->NO;
      uint32_t old_result = wp_ptr->old_result;
      if(str == NULL) {
        printf("Find a null str wp.\n");
        continue;
      } 
      printf("%d  |  %d  |  %s \n", NO, old_result, str);
  }
}

void delete_all_using_wp() {
  bool success = true;
  WP *wp_ptr = head;
  WP *next_wp = head->next;
  for (; wp_ptr != NULL; wp_ptr = next_wp) {
      next_wp = wp_ptr->next;
      free_wp(wp_ptr->NO, &success);
      if(!success) {
        printf("Failed to free watchpoint NO:%d", wp_ptr->NO);
      }
  }
}