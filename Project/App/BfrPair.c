/*--------------- B f r P a i r . c ---------------

by: Michael Nickelson

PURPOSE
Buffer pair functions and definitions

CHANGES
02/19/2014 mn - Initial submission
*/

#include "BfrPair.h"

/*--------------- B f r P a i r I n i t -----------------
Initialize a buffer pair
*/
void BfrPairInit(BfrPair *bfrPair,
                 CPU_INT08U *bfr0Space,
                 CPU_INT08U *bfr1Space,
                 CPU_INT16U size){
  BfrInit(&bfrPair->buffers[0], bfr0Space, size);
  BfrInit(&bfrPair->buffers[1], bfr1Space, size);
  
  return;
}

///*--------------- B f r P a i r I n i t -----------------
//Initialize a buffer pair
//*/
//void BfrPairInit(BfrPair *bfrPair,
//                 CPU_INT16U size){
//  BfrInit(&bfrPair->buffers[0], size);
//  BfrInit(&bfrPair->buffers[1], size);
//  
//  return;
//}

/*--------------- P u t B f r R e s e t -----------------
Reset the put buffer
*/
void PutBfrReset(BfrPair *bfrPair){
  BfrReset(&bfrPair->buffers[bfrPair->putBrfNum]);
  
  return;
}

/*--------------- P u t B f r A d d r -----------------
Return the address of the put buffer
*/
CPU_INT08U *PutBfrAddr(BfrPair *bfrPair){
    return bfrPair->buffers[bfrPair->putBrfNum].buffer;
}

/*--------------- G e t B f r A d d r -----------------
Return the address of the get buffer
*/
CPU_INT08U *GetBfrAddr(BfrPair *bfrPair){
  return bfrPair->buffers[!(bfrPair->putBrfNum)].buffer;
}

/*--------------- P u t B f r C l o s e d -----------------
Return true if the put buffer is closed, otherwise false
*/
CPU_BOOLEAN PutBfrClosed(BfrPair *bfrPair){
  return bfrPair->buffers[bfrPair->putBrfNum].closed;
}

/*--------------- G e t B f r C l o s e d -----------------
Return true if the get buffer is closed, otherwise false
*/
CPU_BOOLEAN GetBfrClosed(BfrPair *bfrPair){
  return bfrPair->buffers[!(bfrPair->putBrfNum)].closed;
}

/*--------------- C l o s e P u t B f r -----------------
Close the put buffer
*/
void ClosePutBfr(BfrPair *bfrPair){
  BfrClose(&bfrPair->buffers[bfrPair->putBrfNum]);
  
  return;
}

/*--------------- O p e n G e t B f r -----------------
Open the get buffer
*/
void OpenGetBfr(BfrPair *bfrPair){
  BfrOpen(&bfrPair->buffers[!(bfrPair->putBrfNum)]);
  
  return;
}

/*--------------- P u t B f r A d d B y t e -----------------
Add a byte to the put buffer. Returns -1 if buffer is full
*/
CPU_INT16S PutBfrAddByte(BfrPair *bfrPair, CPU_INT16S byte){
  CPU_INT16S retVal = BfrAddByte(&bfrPair->buffers[bfrPair->putBrfNum], byte);
  
  return retVal;
}

/*--------------- G e t B f r N e x t B y t e -----------------
Return the next byte from the get buffer without removing it.
*/
CPU_INT16S GetBfrNextByte(BfrPair *bfrPair){
  return BfrNextByte(&bfrPair->buffers[!(bfrPair->putBrfNum)]);
}

/*--------------- G e t B f r R e m B y t e -----------------
Get and remove the next byte from the get buffer
*/
CPU_INT16S GetBfrRemByte(BfrPair *bfrPair){
    return BfrRemoveByte(&bfrPair->buffers[!(bfrPair->putBrfNum)]);
}

/*--------------- B f r P a i r S w a p p a b l e -----------------
Return true if the buffer pair is swappable, otherise false
*/
CPU_BOOLEAN BfrPairSwappable(BfrPair *bfrPair){
  return ((!GetBfrClosed(bfrPair)) & (PutBfrClosed(bfrPair)));
}

/*--------------- B f r P a i r S w a p -----------------
Swap the buffer pair
*/
void BfrPairSwap(BfrPair *bfrPair){
  bfrPair->putBrfNum = !bfrPair->putBrfNum;
  BfrReset(&bfrPair->buffers[bfrPair->putBrfNum]);
  
  return;
}
