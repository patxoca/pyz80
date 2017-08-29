#ifndef _ZEXT_H_
#define _ZEXT_H_

#define MEMORY_SIZE 0x10000
#define MEMORY_MASK 0xFFFF


typedef unsigned char TByte;

typedef struct {
    TByte     memory[MEMORY_SIZE];  /* 64K */
    int       is_done;
    long      cycle_count;
    void     *py_object; /* Puntero al objeto de tipo PyZ80 al que está
                          * asociado el contexto. Es necesario para poder
                          * pasar el emulador en la llamada al sistema.
                          */
} TContext;

extern void SystemCall(TContext *context);

#endif /* _ZEXT_H_ */
