#ifndef _ZEXT_H_
#define _ZEXT_H_

#define MEMORY_SIZE 0x10000
#define MEMORY_MASK 0xFFFF


typedef unsigned char TByte;

typedef struct {
    TByte     memory[MEMORY_SIZE];  /* 64K */
    int       is_done;
} TContext;

#endif /* _ZEXT_H_ */
