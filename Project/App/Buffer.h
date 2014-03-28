/*--------------- B u f f e r . h ---------------

by: Michael Nickelson

PURPOSE
Buffer functions and definitions
Header file

CHANGES
02/19/2014 mn - Initial submission
*/

#ifndef BUFFER_H
#define BUFFER_H

#include "includes.h"
#include "Constants.h"

/*----- t y p e d e f s   u s e d   i n   B u f f e r -----*/
typedef struct
{
  volatile CPU_BOOLEAN closed;
  CPU_INT16U size;
  CPU_INT16U putIndex;
  CPU_INT16U getIndex;
  CPU_INT08U *buffer;
//  BufferSpace_t *buffer;
} Buffer;

/*----- f u n c t i o n    p r o t o t y p e s -----*/
void BfrInit(Buffer * bfr,
             CPU_INT08U *bfrSpace,
             CPU_INT16U size);

void BfrReset(Buffer *bfr);
void BfrClose(Buffer *bfr);
void BfrOpen(Buffer *bfr);
CPU_INT16S BfrAddByte(Buffer *bfr,
                      CPU_INT16S theByte);
CPU_INT16S BfrNextByte(Buffer *bfr);
CPU_INT16S BfrRemoveByte(Buffer *bfr);
CPU_BOOLEAN BfrClosed(Buffer *bfr);
CPU_BOOLEAN BfrEmpty(Buffer *bfr);

#endif