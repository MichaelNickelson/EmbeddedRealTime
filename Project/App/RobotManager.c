/*--------------- R o b o t M a n a g e r . c ---------------

by: Michael Nickelson

PURPOSE
Handle robot packets and responses

CHANGES
04-30-2014 mn -  Initial submission
*/

#include "includes.h"
#include "RobotManager.h"
#include "Constants.h"
#include "assert.h"
#include "Buffer.h"
#include "Error.h"
#include "Framer.h"
#include "PktParser.h"
#include "RobotControl.h"
#include "SerIODriver.h"
#include "string.h"

/*----- c o n s t a n t    d e f i n i t i o n s -----*/

#define LONGEST_PATH 10

#define RobotMgrPrio 4
#define ROBOT_MGR_STK_SIZE 128
#define SUSPEND_TIMEOUT 0

/*----- l o c a l   f u n c t i o n    p r o t o t y p e s -----*/
void RobotMgrTask(void *data);
void ParseReset(void);

/*----- G l o b a l   V a r i a b l e s -----*/
// Task TCB and stack
static OS_TCB robotMgrTCB;
static CPU_STK robotMgrStk[ROBOT_MGR_STK_SIZE];

/*--------------- C r e a t e R o b o t M g r T a s k ---------------
Create/Initialize robot manager task
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
  Buffer *payloadBfr = NULL;
  Payload *payload;
  OS_ERR osErr;
  OS_MSG_SIZE msgSize;
  
  for(;;){
    if(payloadBfr == NULL){
      payloadBfr = OSQPend(&parserQueue,
                   0,
                   OS_OPT_PEND_BLOCKING,
                   &msgSize,
                   NULL,
                   &osErr);
    assert(osErr == OS_ERR_NONE);
    }
    
    payload = (Payload *) payloadBfr->buffer;
    
//    CPU_INT08U x = sizeof(Buffer);
//    CPU_INT32U *p_z = (CPU_INT32U*) payload;
//    CPU_INT32U x = p_z[0];
    
    BfrReset(payloadBfr);
    
    if(payload->dstAddr == MyAddress){ // If message is to me, generate a response
        switch(payload->msgType){
          case(MSG_RESET):
            SendAck(payloadBfr, MSG_RESET);
            ParseReset();
            break;
          case(MSG_ADD):
            AddRobot(payloadBfr);
            break;
          case(MSG_MOVE):
            OSQPost(&robotCtrlQueue[(payload->payloadData.robot.robotAddress) - FIRST_ROBOT],
                    payloadBfr, sizeof(Buffer), OS_OPT_POST_FIFO, &osErr);
//            MoveRobot(payloadBfr);
//            SendAck(payloadBfr, MSG_MOVE);
            break;
          case(MSG_PATH):
            SendAck(payloadBfr, MSG_PATH);
            break;
          case(MSG_LOOP):
            SendAck(payloadBfr, MSG_LOOP);
            break;
          case(MSG_STOP_LOOP):
            SendAck(payloadBfr, MSG_STOP_LOOP);
            break;
          default:  // Handle unknown message types
            SendError(payloadBfr, ERR_MGR_TYPE);
            break;
        }
        payloadBfr = NULL;
    }else if(payload->dstAddr == 0){
      OSQPost(&robotCtrlMbox[(payload->payloadData.robot.robotAddress) - FIRST_ROBOT],
                    payloadBfr, sizeof(Buffer), OS_OPT_POST_FIFO, &osErr);
    }
  }
}

/* Parse and print each message in its own function */

/*--------------- P a r s e R e s e t ---------------
Reset the board
*/
void ParseReset(void){
  NVIC_GenerateCoreReset();
  return;
}