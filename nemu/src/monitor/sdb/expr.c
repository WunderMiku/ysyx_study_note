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

enum {
  TK_NOTYPE = 256, TK_EQ, NUM, TK_U,

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
  {"\\-", '-'},        // minus
  {"\\*", '*'},        // Multiplication
  {"/", '/'},          // Division
  {"\\(", '('},        // Left parenthesis
  {"\\)", ')'},        // right parenthesis
  {"[0-9]+", NUM},     // number
  {"U", TK_U}
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
          case NUM: 
            strncpy(tokens[nr_token].str, substr_start, substr_len);
            tokens[nr_token].str[31] = '\0';  // 确保字符串结束  
            tokens[nr_token].type = rules[i].token_type;
            nr_token++;
            break;
          
          case TK_NOTYPE: break;

          case TK_U: break; // 滤去数字后的U

          default:
            tokens[nr_token].type = rules[i].token_type;
            nr_token++;
        }

        break;
      }
    }

    if (i == NR_REGEX) {
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

// Note：该函数仅会在判断出表达式不合法时修改*success = false
// 若需要借助该函数判断合法性时，需要传入的*success = true
static uint32_t eval(uint32_t p, uint32_t q, bool *success) { 
  Log("eval: p=%d, q=%d", p, q);
  if (p > q) { // 不合法子串
    Log("Invalid substring: p > q");
    *success = false;
    return 0;
  }

  if (p == q) { // 仅存在一个token 若为NUM类型返回其数值，否则不合法
    // 此处假设str数组中为合法的NUM数据
    if(tokens[p].type == NUM) {
      uint32_t val = strtoul(tokens[p].str, NULL, 10);
      Log("Return NUM: %s = %u", tokens[p].str, val);
      return val;
    }
    else { // 不合法
      Log("Single token is not a number: type=%d", tokens[p].type);
      *success = false;
      return 0;
    }
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

  uint32_t val1 = eval(p, op - 1, success);
  uint32_t val2 = eval(op + 1, q, success);

  uint32_t result;
  switch (op_type) {
    case '+': 
      result = val1 + val2;
      Log("Computing: %u + %u = %u", val1, val2, result);
      return result;
    case '-': 
      result = val1 - val2;
      Log("Computing: %u - %u = %u", val1, val2, result);
      return result;
    case '*': 
      result = val1 * val2;
      Log("Computing: %u * %u = %u", val1, val2, result);
      return result;
    case '/': 
      if(val2 == 0) {
        Log("Division by zero error");
        *success = false;
        return 0;
      }
      result = val1 / val2;
      Log("Computing: %u / %u = %u", val1, val2, result);
      return result;

    default: 
      Log("Unknown operator type: %d", op_type);
      assert(0);
  }  
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
  int op_prior = 0; // + - => 1; * / => 0
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

    if(!count) { // 不在括号中的+-*/操作符
      if(T_type == '+' || T_type == '-') {
        op_prior = 1;
        op_pos = i;
      }

      if((T_type == '*' || T_type == '/') && (!op_prior)) op_pos = i;
    }
  }
  return op_pos;
}