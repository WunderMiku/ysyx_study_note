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

#include <assert.h>
#include <isa.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>
#include <stdbool.h>
#include <stdint.h>
#include <memory/vaddr.h>

enum {
  TK_NOTYPE = 256, TK_EQ, NUM, TK_U, HEX_NUM, TK_REGS, TK_NE, TK_AND, DEREF,

  /* TODO: Add more token types */

};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},    // spaces
  {"\\+", '+'},         // plus
  {"==", TK_EQ},        // equal
  {"!=", TK_NE},        // not equal
  {"\\-", '-'},        // minus
  {"\\*", '*'},        // Multiplication
  {"/", '/'},          // Division
  {"\\(", '('},        // Left parenthesis
  {"\\)", ')'},        // right parenthesis
  {"0x[0-9a-fA-F]+", HEX_NUM},  // hexadecimal-number
  {"[0-9]+", NUM},     // number
  {"U", TK_U},         // just U O MY(
  {"\\$[a-z0-9]+", TK_REGS},  // regs
  {"&&", TK_AND}       // and

};

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

static Token tokens[65536] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;
  memset(tokens, 0, sizeof(tokens));

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        switch (rules[i].token_type) {
          case HEX_NUM:
          case NUM: 
            strncpy(tokens[nr_token].str, substr_start, substr_len);
            tokens[nr_token].str[31] = '\0';  // 确保字符串结束  
            tokens[nr_token].type = rules[i].token_type;
            nr_token++;
            break;

          case TK_REGS:
            strncpy(tokens[nr_token].str, substr_start + 1, substr_len - 1);
            tokens[nr_token].str[31] = '\0';  // 确保字符串结束  
            tokens[nr_token].type = rules[i].token_type;
            nr_token++;
            break;
          
          case TK_NOTYPE: break; // 滤去空格

          case TK_U: break; // 滤去数字后的U

          // 如果是表达式的第一个token，或者前一个token不是数字、十六进制数、寄存器或右括号
          // 那么它被识别为解引用操作符（DEREF）
          // 否则为乘号
          case '*':
            if (nr_token == 0 || (tokens[nr_token - 1].type != ')' && tokens[nr_token - 1].type != HEX_NUM && tokens[nr_token - 1].type != NUM \
                 && tokens[nr_token - 1].type != TK_REGS)) {
              tokens[nr_token].type = DEREF;
            } else {
              tokens[nr_token].type = '*';
            }
            nr_token++;
            break;
  
          default:
            tokens[nr_token].type = rules[i].token_type;
            nr_token++;
        }

        break;
      }
    }

    if (i == NR_REGEX) { // 如果到最后都没匹配上
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

static uint32_t eval(uint32_t p, uint32_t q, bool *success);
word_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  return eval(0, nr_token - 1, success);
}

static bool check_parentheses_valid (uint32_t p, uint32_t q);
static bool check_parentheses(uint32_t p, uint32_t q);

static uint32_t get_op_pos(uint32_t p, uint32_t q);


static uint32_t eval(uint32_t p, uint32_t q, bool *success) { 
  *success = true;

  Log("eval: p=%d, q=%d", p, q);
  if (p > q) { // 不合法子串
    Log("Invalid substring: p > q");
    *success = false;
    return 0;
  }

  if (p == q) { // 仅存在一个token 若为 NUM/HEX_NUM 类型返回其数值，如果是寄存器类型 返回其寄存器值 若都不是则不合法
    // 此处认为str数组中为合法的NUM数据
    if((tokens[p].type == NUM) || (tokens[p].type == HEX_NUM)) {
      uint32_t val = strtoul(tokens[p].str, NULL, 0);
      Log("Return NUM: %s = %u", tokens[p].str, val);
      return val;
    }

    if(tokens[p].type == TK_REGS) {
      uint32_t val = (uint32_t) isa_reg_str2val(tokens[p].str, success);
      if(*success) {
      Log("Return Reg: $%s = 0x%08x", tokens[p].str, val);
      return val;
      } else {
        Log("Get regs failed.");
        return 0;
      }
    }

    // 不合法
    Log("Single token is not a number or reg: type=%d", tokens[p].type);
    *success = false;
    return 0;
  
  }

  if (!check_parentheses_valid(p, q)) { // 括号不合法
    Log("Parentheses not valid");
    *success = false;
    return 0;
  }

  if (check_parentheses(p, q)) { // 该表达式被一对括号包括，需要去除该对括号
    Log("Parentheses enclosed, recursing into (%d, %d)", p+1, q-1);
    return eval (p + 1, q - 1, success);
  }

  // 至此，得到了括号合法且不存在最外层括号的表达式子串
  
  uint32_t op = get_op_pos(p, q);
  int op_type = tokens[op].type;
  Log("Operator position: %d, type: %d", op, op_type);
  
  uint32_t val1;
  if (op_type != DEREF) { // 如果是一元运算符 不计算val1
    val1 = eval(p, op - 1, success);
  }

  uint32_t val2 = eval(op + 1, q, success);

  uint32_t result = 0;
  switch (op_type) {
    case '+': 
      result = val1 + val2;
      Log("Computing: %u + %u = %u", val1, val2, result);
      break;

    case '-': 
      result = val1 - val2;
      Log("Computing: %u - %u = %u", val1, val2, result);
      break;

    case '*': 
      result = val1 * val2;
      Log("Computing: %u * %u = %u", val1, val2, result);
      break;

    case '/': 
      if(val2 == 0) {
        Log("Division by zero error");
        *success = false;
        return 0;
      }
      result = val1 / val2;
      Log("Computing: %u / %u = %u", val1, val2, result);
      break;

    case DEREF:
      //检查地址是否越界
      if (val2 < CONFIG_MBASE || val2 > CONFIG_MBASE + CONFIG_MSIZE - 1) { 
        printf("Address " FMT_WORD " is out of bounds. Valid range: [" FMT_WORD ", " FMT_WORD "]\n"\
          ,val2, CONFIG_MBASE, CONFIG_MBASE + CONFIG_MSIZE - 1);
        return 0;
      }
      result = (uint32_t) vaddr_read(val2, 4);
      break;
      
    case TK_EQ:
      result = (val1 == val2);
       Log("Comparing: %u == %u = %s", val1, val2, result ? "true" : "false");
      break;

    case TK_NE:
      result = (val1 != val2);
      Log("Comparing: %u != %u = %s", val1, val2, result ? "true" : "false");
      break;

    case TK_AND:
      result = (val1 && val2);
      Log("Logic AND: %u && %u = %s", val1, val2, result ? "true" : "false");
      break;

    default: 
      Log("Unknown operator type: %d", op_type);
      assert(0);
  }
  Log("Returning result: %u", result);
  return result; 
}

// 检查表达式的括号是否合法
static bool check_parentheses_valid (uint32_t p, uint32_t q) {
  int count = 0;
  for (uint32_t i = p; i <= q; i++) {
    if (tokens[i].type == '(') {
      count++;
    }

    if (tokens[i].type == ')') {
      count--;
    }

    if (count < 0) {
      // 右括号多于左括号，非法表达式
      return false;
    }
  }
  return count ? false : true;
}
static bool check_parentheses(uint32_t p, uint32_t q) {
  int count = 0;
  if (tokens[p].type != '(' || tokens[q].type != ')')
    return false;

  for (uint32_t i = p; i <= q; i++) {
    if (tokens[i].type == '(') {
      count++;
    } else if (tokens[i].type == ')') {
      count--;
      if (count == 0 && i < q) {
        // 中途匹配完了括号，说明不是整个表达式被一对括号包围
        return false;
      }
    }
    if (count < 0) {
      // 右括号多于左括号，非法表达式
      return false;
    }
  }

  // 当且仅当表达式的第一位的'('匹配到最后一位的')'，该表达式满足要求
  return (count == 0);
}

static uint32_t get_op_pos(uint32_t p, uint32_t q) {
  int count = 0;
  int op_pos = 0;
  int op_prior = 10; // 初始最高
  for (int i = p; i <= q; i++) {
    int T_type = tokens[i].type;
    if (T_type == '(') {
      count ++;
      continue;
    }
    if (T_type == ')') {
      count --;
      continue;
    }

    if (T_type == NUM) continue;


    /* Prior:
    *  *(DEREF) #5
    *  * /      #4
    *  + -      #3
    *  == !=    #2
    *  &&       #1  (需要最低优先级的运算符，同优先级时取最后一个)
    */ 
    if(!count) { // 不在括号中的操作符
      if((T_type == DEREF)  && (op_prior >= 5)) {
        op_prior = 5;
        op_pos = i;
      }

      if((T_type == '*' || T_type == '/')  && (op_prior >= 4)) {
        op_prior = 4;
        op_pos = i;
      }

      if((T_type == '+' || T_type == '-')  && (op_prior >= 3)) {
        op_prior = 3;
        op_pos = i;
      }

      if((T_type == TK_EQ || T_type == TK_NE) && (op_prior >= 2)) {
        op_prior = 2;
        op_pos = i;
      }

      if((T_type == TK_AND) && (op_prior >= 1)) {
        op_prior = 1;
        op_pos = i;
      }
    }
  }
  return op_pos;
}