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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>

// this should be enough
static char buf[65536] = {};
static char code_buf[65536 + 128] = {}; // a little larger than `buf`
static char *code_format =
"#include <stdio.h>\n"
"#include <stdint.h>\n"
"int main() { "
"  uint32_t result = %s; "
"  printf(\"%%u\", result); "
"  return 0; "
"}";

static int gen_index = 0;

uint32_t choose(uint32_t n) {
  return rand() % n;
}

void gen (char c) {
  if(gen_index < sizeof(buf)) {
    buf[gen_index ++] = c;
  } else {
    printf("out of buf! \n");
    assert(0);
  }
}
void gen_num_NU (uint32_t max_gen_len) { 
  if(max_gen_len == 0) {
    printf("max_gen_len is invalid! \n");
    return;
  }
  if(max_gen_len != 1) {
    int len = 1 + choose(9);
    len = (max_gen_len < len) ? max_gen_len : len;

    char num = '1' + choose(9); // 避免前导零
    gen(num);

    for (int i = 1; i < len; i++) {
      num = '0' + choose(10);
      gen(num);
    }
  } else { // 仅有一位时
    char num = '0' + choose(9); 
    gen(num);
  }
}

void gen_num (uint32_t max_gen_len) { // 为保证gcc准确使用无符号计算，需要在每一个数字常量后加一个U
  if(max_gen_len < 2) {
    printf("max_gen_len is invalid! \n");
    return;
  }
  if(max_gen_len != 2) {
    int len = 1 + choose(9);
    len = ((max_gen_len - 1) < len) ? (max_gen_len - 1) : len;

    char num = '1' + choose(9); // 避免前导零
    gen(num);

    for (int i = 1; i < len; i++) {
      num = '0' + choose(10);
      gen(num);
    }
  } else { // 仅有两位时（一个数字+U）
    char num = '0' + choose(10); 
    gen(num);
  }
  gen('U');
}

void gen_rand_op() {
  switch (choose(4)) {
    case 0: gen('+'); break;
    case 1: gen('-'); break;
    case 2: gen('*'); break;
    case 3: gen('/'); break;
  }
}

// 生成最大长度为max_gen_len的表达式（不保证有'\0'结尾）
static void gen_rand_expr(uint32_t max_gen_len) {
  if(max_gen_len < 2) {
    printf ("max_gen_len is invalid! \n");
    return;
  }
  
  switch (choose(3)) {
    case 0: 
      gen_num(max_gen_len); 
      break;
    case 1: 
      if(max_gen_len >= 4) {
        gen('('); 
        gen_rand_expr(max_gen_len - 2); 
        gen(')'); 
      } else {
        gen_num(max_gen_len);
      }
      break;
    default: 
      if(max_gen_len >= 5) { 
        gen_rand_expr((max_gen_len - 1) >> 1); 
        gen_rand_op(); 
        gen_rand_expr((max_gen_len - 1) >> 1); 
      } else {
        gen_num(max_gen_len);
      }
      break;
  }
}

int main(int argc, char *argv[]) {
  int seed = time(0);
  srand(seed);
  int loop = 1;
  if (argc > 1) {
    sscanf(argv[1], "%d", &loop);
  }
  int i;
  for (i = 0; i < loop; i ++) {
    gen_index = 0;
    gen_rand_expr(sizeof(buf) - 1);
    gen('\0'); // 保证字符串合法

    sprintf(code_buf, code_format, buf);

    FILE *fp = fopen("/tmp/.code.c", "w");
    assert(fp != NULL);
    fputs(code_buf, fp);
    fclose(fp);

    int ret = system("gcc -Werror=div-by-zero /tmp/.code.c -o /tmp/.expr 2>/dev/null");
    if (ret != 0) continue;

    fp = popen("/tmp/.expr", "r");
    assert(fp != NULL);

    int result;
    ret = fscanf(fp, "%d", &result);
    pclose(fp);

    printf("%u %s\n", result, buf);
  }
  return 0;
}
