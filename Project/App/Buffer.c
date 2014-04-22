/*--------------- B u f f e r . c ---------------

by: Michael Nickelson

PURPOSE
Buffer functions and definitions

CHANGES
02/19/2014 mn - Initial submission
*/

#include "Buffer.h"

/*--------------- B f r I n i t -----------------
Initialize a buffer
*/
void BfrInit(Buffer *bfr, CPU_INT08U *bfrSpace, CPU_INT16U size){
  bfr->size = size;
  bfr->buffer = bfrSpace;
  BfrReset(bfr);
  
  return;
}

/*--------------- B f r R e s e t -----------------
Reset a buffer
*/
void BfrReset(Buffer *bfr){
  bfr->closed = FALSE;
  bfr->putIndex = 0;
  bfr->getIndex = 0;
  
  return;
}

/*--------------- B f r C l o s e d -----------------
Return true if the buffer is closed, otherwise false
*/
CPU_BOOLEAN BfrClosed(Buffer *bfr){
  return bfr->closed;
}

/*--------------- B f r C l o s e -----------------
Close the buffer
*/
void BfrClose(Buffer *bfr){
  bfr->closed = TRUE;
  
  return;
}

/*--------------- B f r O p e n -----------------
Open the buffer
*/
void BfrOpen(Buffer *bfr){
  bfr->closed = FALSE;
  
  return;
}

/*--------------- B f r E m p t y-----------------
Retur true if the buffer is empty, otherwise false
*/
CPU_BOOLEAN BfrEmpty(Buffer *bfr){
  return (bfr->getIndex >= bfr->putIndex);
}

/*--------------- B f r A d d B y t e-----------------
Add a byte to the given buffer and return the byte. If the buffer is full 
return -1. Close the buffer if it becomes full.
*/
CPU_INT16S BfrAddByte(Buffer *bfr, CPU_INT16S theByte){
  CPU_INT16S retVal = -1;
  
  if(!BfrClosed(bfr)){
    retVal = theByte;
    bfr->buffer[bfr->putIndex++] = theByte;
//    bfr->putIndex++;
  }
  
  if(bfr->putIndex >= bfr->size)
    BfrClose(bfr);
  
  return retVal;
}

/*--------------- B f r N e x t B y t e -----------------
Return the next byte from the buffer without removing it.
*/
CPU_INT16S BfrNextByte(Buffer *bfr){
  CPU_INT16S retVal = -1;
  
  if(!BfrEmpty(bfr)){
    retVal = bfr->buffer[bfr->getIndex];
  }
  
  return retVal;
}

/*--------------- B f r R e m B y t e -----------------
Get and remove the next byte from the buffer
*/
CPU_INT16S BfrRemoveByte(Buffer *bfr){
  CPU_INT16S retVal = -1;
  
  if(!BfrEmpty(bfr)){
    retVal = bfr->buffer[bfr->getIndex];
    bfr->getIndex++;
  }
  
  if(BfrEmpty(bfr))
    bfr->closed = FALSE;
  
  return retVal;
}