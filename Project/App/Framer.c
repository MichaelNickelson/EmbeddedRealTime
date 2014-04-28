/*--------------- F r a m e r . c ---------------

by: Michael Nickelson

PURPOSE
Handles sending outgoing packets to oBfrPair.

CHANGES
04-30-2014 - mn - Initial Submission
*/

/* Include dependencies */
#include "includes.h"
#include "Framer.h"
#include "assert.h"
#include "BfrPair.h"
#include "MemMgr.h"
#include "RobotMgr.h"
#include "SerIODriver.h"

/*----- c o n s t a n t    d e f i n i t i o n s -----*/
#define SUSPEND_TIMEOUT 0
#define FRAMER_STK_SIZE 128
#define FramerPrio 3

#define PAYLOAD_SIZE 5

/*----- G l o b a l   V a r i a b l e s -----*/
OS_TCB framerTCB;
CPU_STK framerStk[FRAMER_STK_SIZE];

OS_Q framerQueue; // The queue of parsed packets to RobotManager

/*----- l o c a l   f u n c t i o n    p r o t o t y p e s -----*/
void Framer(void *data);

/*--------------- C r e a t e F r a m e r T a s k ---------------
Start the packet parsing task and create the relevant semaphores
*/
void CreateFramerTask(void){
  OS_ERR osErr;
  
  
  OSQCreate(&framerQueue, "Framer Queue", PoolSize, &osErr);
  assert(osErr == OS_ERR_NONE);
  
  // Start framer task and verify success
  OSTaskCreate(&framerTCB,
               "Packet framing task",
               Framer,
               NULL,
               FramerPrio,
               &framerStk[0],
               FRAMER_STK_SIZE / HIGH_WATER_LIMIT,
               FRAMER_STK_SIZE,
               0,
               0,
               (void *)0,
               0,
               &osErr);
  assert(osErr == OS_ERR_NONE);
}

void Framer(void *data){
  OS_ERR osErr;
  OS_MSG_SIZE msgSize;
  Buffer *payloadBfr = NULL;
  CPU_INT08U c;
  CPU_INT08U cs;
  BfrPair oBfrPair;
  
  CPU_INT08U preamble[PREAMBLE_LENGTH] = {0x03, 0xAF, 0xEF};
  
  for(;;){
    
    if(payloadBfr == NULL){
      payloadBfr = OSQPend(&framerQueue,
                           SUSPEND_TIMEOUT,
                           OS_OPT_PEND_BLOCKING,
                           &msgSize,
                           NULL,
                           &osErr);
      assert(osErr == OS_ERR_NONE);
    }
    
    cs = 0;
    
    for(CPU_INT08U j = 0;j<PREAMBLE_LENGTH;j++){
      PutByte(preamble[j]);
      cs ^= preamble[j];
    }
    while(BfrClosed(payloadBfr)){
      c = BfrRemoveByte(payloadBfr);
      cs ^= c;
      PutByte(c);
    }
    PutByte(cs);
    BfrFlush();
  
    Free(payloadBfr);
    payloadBfr = NULL;
  }
}

/*--------------- S e n d A c k ---------------
Send acknowledgement message to Framer
*/
void SendAck(CPU_INT08U type){
  Buffer *ackBfr = Allocate();  
  
  // Send an Ack packet to the framer for transmission
  MakePayload(ackBfr, CtrlCtrAddress, MSG_ACK, type);
}

/*--------------- M a k e P a y l o a d ---------------
Make a payload and send it to the framer.
*/
void MakePayload(Buffer *payloadBfr, CPU_INT08U receiver, CPU_INT08U type, CPU_INT08U payload){
  OS_ERR osErr;
  
  BfrReset(payloadBfr);
  BfrAddByte(payloadBfr, PAYLOAD_SIZE + PREAMBLE_LENGTH + 1);
  BfrAddByte(payloadBfr, receiver);
  BfrAddByte(payloadBfr, MyAddress);
  BfrAddByte(payloadBfr, type);
  BfrAddByte(payloadBfr, payload);
  BfrClose(payloadBfr);
  
  OSQPost(&framerQueue, payloadBfr, sizeof(Buffer), OS_OPT_POST_FIFO, &osErr);
  assert(osErr == OS_ERR_NONE);
}