#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

int printf(const char *fmt, ...) {
  panic("Not implemented");
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  panic("Not implemented");
}

int sprintf(char *out, const char *fmt, ...) { 
  va_list ap;
  int d, ret = 0;
  char *s;
  char buf[16];

  va_start(ap, fmt);
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
          d = -d;
        }
        
        int i = 0;
        while(d) {
          buf[i++] = (d % 10) + '0';
          d /= 10;
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
        ret--; // for the last '\0' added
        break;

      case '%':
        *out++ = '%';
        ret++;
        break;

      default: assert(0);
    }
  }
  *out = 0; // add '\0'
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
