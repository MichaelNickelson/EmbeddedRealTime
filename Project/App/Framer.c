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
//#include "Error.h"
#include "MemMgr.h"
#include "SerIODriver.h"

/*----- c o n s t a n t    d e f i n i t i o n s -----*/
#define SUSPEND_TIMEOUT 250
#define FRAMER_STK_SIZE 128
#define FramerPrio 4
#define HIGH_WATER_LIMIT 10
#define PREAMBLE_LENGTH 3

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
  
  CPU_INT08U preamble[PREAMBLE_LENGTH] = {0x03, 0xAF, 0xEF};
  
  for(;;){
    
    if(payloadBfr == NULL){
      payloadBfr = OSQPend(&framerQueue,
                           0,
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
    
    Free(payloadBfr);
    payloadBfr = NULL;
  }
}