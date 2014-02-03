#include "Buffer.h"

void BfrInit(Buffer *bfr, CPU_INT08U *bfrSpace, CPU_INT16U size){
  bfr->size = size;
  bfr->buffer = bfrSpace;
  BfrReset(bfr);
  
  return;
}

void BfrReset(Buffer *bfr){
  bfr->closed = FALSE;
  bfr->putIndex = 0;
  bfr->getIndex = 0;
  
  return;
}

CPU_BOOLEAN BfrClosed(Buffer *bfr){
  return bfr->closed;
}

void BfrClose(Buffer *bfr){
  bfr->closed = TRUE;
  
  return;
}

void BfrOpen(Buffer *bfr){
  bfr->closed = FALSE;
  
  return;
}

CPU_BOOLEAN BfrEmpty(Buffer *bfr){
  return (bfr->getIndex >= bfr->putIndex);
}

CPU_INT16S BfrAddByte(Buffer *bfr, CPU_INT16S theByte){
  CPU_INT16S retVal = theByte;
  
  if(BfrClosed(bfr)){
    retVal = -1;
  }else{
    bfr->buffer[bfr->putIndex] = theByte;
    bfr->putIndex++;
  }
  
  if(bfr->putIndex >= bfr->size)
    BfrClose(bfr);
  
  return retVal;
}

CPU_INT16S BfrNextByte(Buffer *bfr){
  CPU_INT16S retVal = -1;
  
  if(!BfrEmpty(bfr)){
    retVal = bfr->buffer[bfr->getIndex];
  }
  
  return retVal;
}

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