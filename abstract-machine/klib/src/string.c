#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
  const char *p = s;
  while (*p) p++;
  return p - s;
}

char *strcpy(char *dst, const char *src) {
  char *ret = dst;
  while ((*dst++ = *src++));
  return ret;
}

char *strncpy(char *dst, const char *src, size_t n) {
  char *ret = dst;
  while (n && (*dst++ = *src++)) n--;
  if(n != 0) n--; // 如果因\0而终止，n会少减一次
  while (n--) *dst++ = 0;
  return ret;
}

char *strcat(char *dst, const char *src) {
  char *ret = dst;
  while (*dst) dst++;
  while ((*dst++ = *src++));
  return ret;
}

int strcmp(const char *s1, const char *s2) {
  while(*s1 && (*s1 == *s2)) {
    s1++;
    s2++;
  }
  return *(unsigned char *)s1 - *(unsigned char *)s2;
}

int strncmp(const char *s1, const char *s2, size_t n) {
  while (n && *s1 && (*s1 == *s2)) {
    s1++;
    s2++;
    n--;
  }
  if (n == 0) return 0;
  return *(unsigned char *)s1 - *(unsigned char *)s2;
}

void *memset(void *s, int c, size_t n) {
  char *p = s;
  while (n--) {
    *p++ = (char)c;
  }
  return s;
}

void *memmove(void *dst, const void *src, size_t n) {
  char *d = dst;
  const char *s = src;
  if (d <= s || d >= s + n) {  // 目标区域在源区域前面,重合或在后面但不重叠
    while (n--) {
      *d++ = *s++;
    }
  } else {
    d += n;
    s += n;
    while (n--) {
      *(--d) = *(--s);
    }
  }
  return dst;
}

void *memcpy(void *out, const void *in, size_t n) {
  char *d = out;
  const char *s = in;
  while (n--) {
    *d++ = *s++;
  }
  return out;
}

int memcmp(const void *s1, const void *s2, size_t n) {
  const unsigned char *p1 = s1, *p2 = s2;
  while (n-- && *p1 == *p2) {
    p1++;
    p2++;
  }
  if (n == (size_t)-1) return 0; 
  return *p1 - *p2;
}

#endif
