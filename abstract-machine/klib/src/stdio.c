#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)
#define WIDTH_BUF_SIZE 16

typedef enum {
  DEC = 0,
  HEX_little,
  HEX_big,
  OCT
} BASE;

typedef union {
  unsigned int u;
  int i;
} number_t;
#define FMT_SIGNED(x)  ((number_t){.i = (x)})
#define FMT_UNSIGNED(x) ((number_t){.u = (x)})

// 获取字符串表示的整数,返回int值
static int stoi(char *s, BASE base) {
  panic_on(s == NULL, "Input string is NULL!");

  bool neg = false;

  unsigned int uret = 0;
  int ret = 0;

  switch(base) {
    case DEC:
      if(*s == '-') {
        neg = true;
        s++;
      }

      while (*s >= '0' && *s <= '9') {
        uret = uret * 10 + (*s - '0');
        s++;
      }
    break;

    case HEX_little:
    case HEX_big:
      while ((*s >= '0' && *s <= '9') ||
             (*s >= 'a' && *s <= 'f') ||
             (*s >= 'A' && *s <= 'F')) {
        if(*s >= '0' && *s <= '9') {
          uret = uret * 16 + (*s - '0');
        } else if(*s >= 'a' && *s <= 'f') {
          uret = uret * 16 + (*s - 'a' + 10);
        } else {
          uret = uret * 16 + (*s - 'A' + 10);
        }
        s++;
      }
    break;

    case OCT:
    while (*s >= '0' && *s <= '7') {
      uret = uret * 8 + (*s - '0');
      s++;
    }
    break;

    default:
      panic("Unsupported base!");
  }
  assert(uret <= INT32_MAX && -uret >= INT32_MIN); // 防止溢出 
    ret = neg ? -uret : uret;
  return ret;
}

// 在out开始追加无符号d的字符串表示，并添加结束符 \0
// 函数返回添加的数字字符数（不包括 \0）
static int itous(unsigned int ud, char *out, BASE base) {
  panic_on(out == NULL, "Output buffer is NULL!");

  int ret = 0;
  int base_num;
  char buf[WIDTH_BUF_SIZE];
  base_num = (base == DEC) ? 10 : (base == HEX_little || base == HEX_big) ? 16 : (base == OCT) ? 8 : -1;
  panic_on(base_num == -1, "Unsupported base!");

  if(ud == 0) {
    *out++ = '0';
    ret++;
    return ret;
  } 
  
  int i = 0;
  while(ud) {
    int rem = ud % base_num;
    if(rem < 10) {
      buf[i++] = rem + '0';
    } else {
      buf[i++] = (base == HEX_little ? (rem - 10 + 'a') : (rem - 10 + 'A'));
    }
    ud /= base_num;
    assert(i < WIDTH_BUF_SIZE);
  }
  while(i--) {
    *out++ = buf[i];
    ret++;
  }
  *out = 0; // add '\0'
  return ret;
}

// 在out开始追加有符号d的字符串表示，并添加结束符 \0
// 返回添加的字符数（不包括 \0）
static int itos(int d, char *out, BASE base) {
  panic_on(out == NULL, "Output buffer is NULL!");
  panic_on(base != DEC, "Use wrong function to print non-decimal int!");

  int ret = 0;
  unsigned int ud;
  int base_num;
  base_num = (base == DEC) ? 10 : (base == HEX_little || base == HEX_big) ? 16 : (base == OCT) ? 8 : -1;
  panic_on(base_num == -1, "Unsupported base!");

  if(d == 0) {
    *out++ = '0';
    ret++;
    return ret;
  } 
  
  if(d < 0) {
    *out++ = '-';
    ret++;
    ud = -d; // 避免INT_MIN的问题（int不能表示-INT_MIN）
  } else {
    ud = d;
  }
  
  ret += itous(ud, out, base);  // 复用无符号整数的函数

  return ret;
}

static int fmt_print(number_t num, char *out, BASE base, bool filled_zeros, int width, bool signed_int) {
  panic_on(out == NULL, "Output buffer is NULL!");
  panic_on(signed_int && base != DEC, "Only decimal base is supported for signed integers!");

  int ret = 0;
  char buf[WIDTH_BUF_SIZE];
  int offest;
  if(signed_int) {offest = itos(num.i, buf, base);}
  else {offest = itous(num.u, buf, base);}
  
  int append_int = width - offest;
  while(append_int > 0) {
    if(filled_zeros) {
      *out++ = '0';
    } else {
      *out++ = ' ';
    }
    ret++;
    append_int--;
  }
  for (int i = 0; i < offest; i++) {
    *out++ = buf[i];
    ret++;
  }

  return ret;

}

int vsprintf(char *out, const char *fmt, va_list ap) {
  panic_on(out == NULL, "Output buffer is NULL!");
  panic_on(fmt == NULL, "Format string is NULL!");

  int d, ret = 0;
  char *s;
  bool filled_zeros = false;
  int width = 0;
  char width_buf[WIDTH_BUF_SIZE] = "";

  while(*fmt) {
    if(*fmt != '%') {
      *out++ = *fmt++;
      ret++;
      continue;
    } 

    fmt++;  // skip '%'
    if (*fmt == '\0') break; // expect "END by %\0"
    
    // 填零处理
    if(*fmt == '0') {
      filled_zeros = true;
      fmt++;
    }
    // width 处理
    int width_i = 0;
    if(*fmt >= '1' && *fmt <= '9') {
      while(*fmt >= '0' && *fmt <= '9') {
        width_buf[width_i++] = *fmt++;
        assert(width_i < WIDTH_BUF_SIZE);
      }
    }
    width_buf[width_i] = 0; // add '\0'，保证字符串结束
    if(width_i > 0) {
      width = stoi(width_buf, DEC);
    }
    width_i = 0;
    assert(width >= 0);

    int offest;
    switch(*fmt++) {
      case 'd':
        d = va_arg(ap, int);

        offest = fmt_print(FMT_SIGNED(d), out, DEC, filled_zeros, width, true);
        out += offest;
        ret += offest;

        filled_zeros = false;
        width = 0;
        width_buf[0] = 0;
        break;

      case 'u':
        d = va_arg(ap, unsigned int);

        offest = fmt_print(FMT_UNSIGNED(d), out, DEC, filled_zeros, width, false);
        out += offest;
        ret += offest;

        filled_zeros = false;
        width = 0;
        width_buf[0] = 0;
        break;
      
      case 'x':
        d = va_arg(ap, unsigned int);

        offest = fmt_print(FMT_UNSIGNED(d), out, HEX_little, filled_zeros, width, false);
        out += offest;
        ret += offest;

        filled_zeros = false;
        width = 0;
        width_buf[0] = 0;
        break;
      
      case 'X':
        d = va_arg(ap, unsigned int);

        offest = fmt_print(FMT_UNSIGNED(d), out, HEX_big, filled_zeros, width, false);
        out += offest;
        ret += offest;
        
        filled_zeros = false;
        width = 0;
        width_buf[0] = 0;
        break;

      case 's':
        s = va_arg(ap, char *);
        if(s == NULL) { 
           s = "(null)";
        }
        int append_str = width - strlen(s);
        if(append_str > 0) {
          while(append_str > 0) {
            *out++ = ' ';
            append_str--;
            ret++;
          }
        }
        while ((*out++ = *s++)) {
          ret++;
        };
        out--, ret--;  // delete the last '\0'
        break;
        
      case 'c':
        *out++ = va_arg(ap, int);
        ret++;
        filled_zeros = false;
        width = 0;
        width_buf[0] = 0;
        break;
        
      case '%':
        *out++ = '%';
        ret++;
        break;

      default:
      printf("Unsupported format string: %c\n", *fmt);
      // assert(0);
      *out = 0;
      return ret;
    }
  }
  *out = 0; // add '\0'
  return ret;
}


int printf(const char *fmt, ...) {
  char out[1024];
  va_list ap;
  va_start(ap, fmt);
  int ret = vsprintf(out, fmt, ap);
  va_end(ap);

  for (char *p = out; *p; p++) {
    putch(*p);
  }

  return ret;
}

int sprintf(char *out, const char *fmt, ...) { 
  va_list ap;
  va_start(ap, fmt);
  int ret = vsprintf(out, fmt, ap);
  va_end(ap);
  return ret;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  panic("Not implemented");
}

#endif
