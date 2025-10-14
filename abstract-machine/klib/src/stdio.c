#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

int vsprintf(char *out, const char *fmt, va_list ap) {
  int d, ret = 0;
  unsigned int ud;
  char *s;
  char buf[16];

  while(*fmt) {
    if(*fmt != '%') {
      *out++ = *fmt++;
      ret++;
      continue;
    } 

    fmt++;  // skip '%'
    if (*fmt == '\0') break; // expect: "...%"

    switch(*fmt++) {
      case 'd':
        d = va_arg(ap, int);
        if(d == 0) {
          *out++ = '0';
          ret++;
          break;
        } 
        
        if(d < 0) {
          *out++ = '-';
          ret++;
          ud = -d; // 避免INT_MIN的问题（int不能表示-INT_MIN）
        } else {
          ud = d;
        }
        
        int i = 0;
        while(ud) {
          buf[i++] = (ud % 10) + '0';
          ud /= 10;
        }
        while(i--) {
          *out++ = buf[i];
          ret++;
        }
        break;

      case 's':
        s = va_arg(ap, char *);
        if(s == NULL) { 
           s = "(null)";
        }
        while ((*out++ = *s++)) {
          ret++;
        };
        out--, ret--;  // delete the last '\0'
        break;

      case '%':
        *out++ = '%';
        ret++;
        break;

      default: assert(0);
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
