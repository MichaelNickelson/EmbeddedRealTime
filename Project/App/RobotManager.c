/*--------------- R o b o t M a n a g e r . c ---------------

by: Michael Nickelson

PURPOSE
Handle robot packets and responses

CHANGES
04-30-2014 mn -  Initial submission
*/

#include "includes.h"
#include "RobotManager.h"
#include "assert.h"
#include "Buffer.h"
#include "Error.h"
#include "Framer.h"
#include "PktParser.h"
#include "SerIODriver.h"
#include "string.h"

/*----- c o n s t a n t    d e f i n i t i o n s -----*/
#define MyAddress 2

#define RobotMgrBfrSize 24
#define LONGEST_PATH 10

#define RobotMgrPrio 4
#define ROBOT_MGR_STK_SIZE 128
#define SUSPEND_TIMEOUT 0
#define HIGH_WATER_LIMIT 10

/*-----  Assign easy to read names to message ID -----*/
#define MSG_RESET 0
#define MSG_ADD 1
#define MSG_MOVE 2
#define MSG_PATH 3
#define MSG_LOOP 4
#define MSG_STOP_LOOP 5

/*----- t y p e d e f s   u s e d   i n   p a y l o a d   m o d u l e -----*/
#pragma pack(1)
typedef struct
{
  CPU_INT08S    payloadLen;
  CPU_INT08U    dstAddr;
  CPU_INT08U    srcAddr;
  CPU_INT08U    msgType;
  CPU_INT08U    payloadData[2*LONGEST_PATH];
} Payload;

typedef enum { P, R } PayloadState;

/*----- l o c a l   f u n c t i o n    p r o t o t y p e s -----*/
void RobotMgrTask(void *data);
void ParseReset(Payload *payload, Buffer payloadBfr);
void SendAck(Buffer *pBfr, CPU_INT08U type);

/*----- G l o b a l   V a r i a b l e s -----*/
// Task TCB and stack
static OS_TCB robotMgrTCB;
static CPU_STK robotMgrStk[ROBOT_MGR_STK_SIZE];

/*--------------- C r e a t e R o b o t M g r T a s k ---------------
Create/Initialize payload buffer pair and start the payload task
*/
void CreateRobotMgrTask(void){
  OS_ERR osErr;
  
  // Create the payload task
  OSTaskCreate(&robotMgrTCB,
               "Robot manager task",
               RobotMgrTask,
               NULL,
               RobotMgrPrio,
               &robotMgrStk[0],
               ROBOT_MGR_STK_SIZE / HIGH_WATER_LIMIT,
               ROBOT_MGR_STK_SIZE,
               0,
               0,
               (void *)0,
               0,
               &osErr);
  
  assert(osErr == OS_ERR_NONE);
}

/*--------------- R o b o t M g r T a s k ---------------
Get a payload from the packet parser and forward it to the appropriate robot
controller.
*/
void RobotMgrTask(void *data){
  CPU_BOOLEAN replyDone = FALSE;
  static PayloadState pState = P;
//  static CPU_CHAR reply[ReplyBfrSize];
  Buffer *pBfr = NULL;
  Payload *payload;
  OS_ERR osErr;
  OS_MSG_SIZE msgSize;
  
  for(;;){
    if(pState == P){ // If no reply is being sent check payload data ready conditions
      // Wait here for a payload buffer to close
//      OSSemPend(&closedPayloadBfrs, SUSPEND_TIMEOUT, OS_OPT_PEND_BLOCKING, NULL, &osErr);
//      assert(osErr==OS_ERR_NONE);
      
    if(pBfr == NULL){
      pBfr = OSQPend(&parserQueue,
                   0,
                   OS_OPT_PEND_BLOCKING,
                   &msgSize,
                   NULL,
                   &osErr);
    assert(osErr == OS_ERR_NONE);
    }
    
    payload = (Payload *) pBfr->buffer;
    
    BfrReset(pBfr);
    
    if(payload->dstAddr == MyAddress){ // If message is to me, generate a response
        switch(payload->msgType){
          case(MSG_RESET):
            SendAck(pBfr, MSG_RESET);
            ParseReset(payload, *pBfr);
            break;
          case(MSG_ADD):
            SendAck(pBfr, MSG_ADD);
            break;
          case(MSG_MOVE):
            SendAck(pBfr, MSG_MOVE);
            break;
          case(MSG_PATH):
            SendAck(pBfr, MSG_PATH);
            break;
          case(MSG_LOOP):
            SendAck(pBfr, MSG_LOOP);
            break;
          case(MSG_STOP_LOOP):
            SendAck(pBfr, MSG_STOP_LOOP);
            break;
          default:  // Handle unknown message types
            SendError(ERR_MGR_TYPE, pBfr);
            break;
        }
        pBfr = NULL;
    }else if(payload->dstAddr == 0){
      SendAck(pBfr, MSG_ADD);                                                   // This is not right!!!!!!!!!!!!!!!!!
    }
//      pState = R;
//      OpenGetBfr(&payloadBfrPair);
//      OSSemPost(&openPayloadBfrs, OS_OPT_POST_1, &osErr);
//      assert(osErr==OS_ERR_NONE);
//      if(BfrPairSwappable(&payloadBfrPair))
//            BfrPairSwap(&payloadBfrPair);
    }else{
//      replyDone = SendReply(reply);
      pState = replyDone ? P : R;
    }
  }
}

/* Parse and print each message in its own function */

/*--------------- P a r s e R e s e t ---------------
Reset the board
*/
void ParseReset(Payload *payload, Buffer payloadBfr){
  NVIC_GenerateCoreReset();
  return;
}

/*--------------- S e n d A c k ---------------
Send acknowledgement message to Framer
*/
void SendAck(Buffer *pBfr, CPU_INT08U type){
  OS_ERR osErr;
  
  // Send an error packet to the framer for transmission
  BfrAddByte(pBfr, 9);
  BfrAddByte(pBfr, 1);
  BfrAddByte(pBfr, 2);
  BfrAddByte(pBfr, 0x0A);
  BfrAddByte(pBfr, type);
  
  BfrClose(pBfr);
  OSQPost(&framerQueue, pBfr, sizeof(Buffer), OS_OPT_POST_FIFO, &osErr);
  assert(osErr == OS_ERR_NONE);
}