/*--------------- B f r P a i r . h ---------------

by: Michael Nickelson

PURPOSE
Buffer pair functions and definitions
Header file

CHANGES
02/19/2014 mn - Initial submission
*/

#ifndef BFRPAIR_H
#define BFRPAIR_H

#include "includes.h"
#include "Buffer.h"

/*----- c o n s t a n t    d e f i n i t i o n s -----*/
#define NumBfrs 2

/*----- t y p e d e f s   u s e d   i n   B f r P a i r -----*/
typedef struct
{
  CPU_INT08U putBrfNum;
  Buffer buffers[NumBfrs];
} BfrPair;

/*----- f u n c t i o n    p r o t o t y p e s -----*/
void BfrPairInit(BfrPair *bfrPair,
                 CPU_INT08U *bfr0Space,
                 CPU_INT08U *bfr1Space,
                 CPU_INT16U size);

void PutBfrReset(BfrPair *bfrPair);
void ClosePutBfr(BfrPair *bfrPair);
void OpenGetBfr(BfrPair *bfrPair);
void BfrPairSwap(BfrPair *bfrPair);
CPU_INT08U *PutBfrAddr(BfrPair *bfrPair);
CPU_INT08U *GetBfrAddr(BfrPair *bfrPair);
CPU_INT16S PutBfrAddByte(BfrPair *bfrPair,
                         CPU_INT16S byte);
CPU_INT16S GetBfrNextByte(BfrPair *bfrPair);
CPU_INT16S GetBfrRemByte(BfrPair *bfrPair);
CPU_BOOLEAN BfrPairSwappable(BfrPair *bfrPair);
CPU_BOOLEAN PutBfrClosed(BfrPair *bfrPair);
CPU_BOOLEAN GetBfrClosed(BfrPair *bfrPair);

#endif