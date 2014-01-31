#ifndef BUFFER_H
#define BUFFER_H
/*--------------- B u f f e r . h ---------------*/

#include "includes.h"

typedef struct
{
  CPU_BOOLEAN closed;
  CPU_INT16U size;
  CPU_INT16U putIndex;
  CPU_INT16U getIndex;
  CPU_INT08U *buffer;
} Buffer;

void BfrInit(Buffer * bfr,
             CPU_INT08U *bfrSpace,
             CPU_INT16U size);

void BfrReset(Buffer *bfr);

CPU_BOOLEAN BfrClosed(Buffer *bfr);

void BfrClose(Buffer *bfr);

void BfrOpen(Buffer *bfr);

CPU_BOOLEAN BfrEmpty(Buffer *bfr);

CPU_INT16S BfrAddByte(Buffer *bfr,
                      CPU_INT16S theByte);

CPU_INT16S BfrNextByte(Buffer *bfr);

CPU_INT16S BfrRemoveByte(Buffer *bfr);

#endif