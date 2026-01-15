#ifndef __SDB_H__
#define __SDB_H__ 

#include <stdint.h>
void sdbMainLoop();
uint32_t expr(char *e, bool *success);
void init_regex();

#endif