#include "BfrPair.h"

void BfrPairInit(BfrPair *bfrPair,
                 CPU_INT08U *bfr0Space,
                 CPU_INT08U *bfr1Space,
                 CPU_INT16U size){
//  Buffer* bfr = &bfrPair->buffers[0];
//  BfrInit(bfr,bfr0Space, size);
  BfrInit(&bfrPair->buffers[0], bfr0Space, size);
  BfrInit(&bfrPair->buffers[1], bfr1Space, size);
  
  return;
}

void PutBfrReset(BfrPair *bfrPair){
  BfrReset(&bfrPair->buffers[bfrPair->putBrfNum]);
  
  return;
}

CPU_INT08U *PutBfrAddr(BfrPair *bfrPair){
    return bfrPair->buffers[bfrPair->putBrfNum].buffer;
}

CPU_INT08U *GetBfrAddr(BfrPair *bfrPair){
  return bfrPair->buffers[!(bfrPair->putBrfNum)].buffer;
}

CPU_BOOLEAN PutBfrClosed(BfrPair *bfrPair){
  return bfrPair->buffers[bfrPair->putBrfNum].closed;
}

CPU_BOOLEAN GetBfrClosed(BfrPair *bfrPair){
  return bfrPair->buffers[!(bfrPair->putBrfNum)].closed;
}

void ClosePutBfr(BfrPair *bfrPair){
  BfrClose(&bfrPair->buffers[bfrPair->putBrfNum]);
  
  return;
}

void openGetBfr(BfrPair *bfrPair){
  BfrOpen(&bfrPair->buffers[!(bfrPair->putBrfNum)]);
  
  return;
}

CPU_INT16S PutBfrAddByte(BfrPair *bfrPair,
                         CPU_INT16S byte){
  CPU_INT16S retVal = BfrAddByte(&bfrPair->buffers[bfrPair->putBrfNum], byte);
  
  return retVal;
}

CPU_INT16S GetBfrNextByte(BfrPair *bfrPair){
  CPU_INT16S retVal = BfrNextByte(&bfrPair->buffers[!(bfrPair->putBrfNum)]);
  
  return retVal;
}

CPU_INT16S GetBfrRemByte(BfrPair *bfrPair){
    CPU_INT16S retVal = BfrRemoveByte(&bfrPair->buffers[!(bfrPair->putBrfNum)]);

    return retVal;
}

CPU_BOOLEAN BfrPairSwappable(BfrPair *bfrPair){
  return ((!GetBfrClosed(bfrPair)) & (PutBfrClosed(bfrPair)));
}

void BfrPairSwap(BfrPair *bfrPair){
  bfrPair->putBrfNum = !bfrPair->putBrfNum;
  BfrReset(&bfrPair->buffers[bfrPair->putBrfNum]);
  
  return;
}
