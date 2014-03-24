/*--------------- R o b o t C t r l . c ---------------

by: Michael Nickelson

PURPOSE
Manage the stepping of individual robots

CHANGES
04-30-2014 mn -  Initial submission
*/

#include "includes.h"
#include "RobotControl.h"
#include "assert.h"
#include "Constants.h"
#include "Error.h"
#include "Framer.h"
#include "MemMgr.h"
#include "RobotManager.h"

#define RobotCtrlPrio 4
#define ROBOT_CTRL_STK_SIZE 128

typedef struct
{
  Coord_t location;
  CPU_BOOLEAN exists;
} Robot_t;


/*----- l o c a l   f u n c t i o n    p r o t o t y p e s -----*/
void RobotCtrlTask(void *data);
void MoveRobot(Buffer *payloadBfr);

/*----- G l o b a l   V a r i a b l e s -----*/
// Task TCB and stack
static OS_TCB robotCtrlTCB[MAX_ROBOTS];
static CPU_STK robotCtrlStk[ROBOT_CTRL_STK_SIZE];
OS_Q robotCtrlMbox[MAX_ROBOTS];
OS_Q robotCtrlQueue[MAX_ROBOTS];

OS_SEM messageWaiting[MAX_ROBOTS];

// Array of robots for collision and existence detection.
static Robot_t robots[MAX_ROBOTS];

/*--------------- C r e a t e R o b o t C t r l T a s k ---------------
Create/Initialize robot controller task
*/
void CreateRobotCtrlTask(CPU_INT08U id){
  OS_ERR osErr;
  
//  OSSemCreate(&messageWaiting[id-FIRST_ROBOT], "Message waiting", 0, &osErr);
//  assert(osErr == OS_ERR_NONE);
  
  OSQCreate(&robotCtrlMbox[id-FIRST_ROBOT], "Robot Mailbox", 1, &osErr);
  assert(osErr == OS_ERR_NONE);
  
  OSQCreate(&robotCtrlQueue[id-FIRST_ROBOT], "Robot Queue", PoolSize, &osErr);
  assert(osErr == OS_ERR_NONE);
  
  // Create the payload task
  OSTaskCreate(&robotCtrlTCB[id-FIRST_ROBOT],
               "Robot controller task",
               RobotCtrlTask,
               //NULL,  // make this take id?
               &id,
               RobotCtrlPrio,
               &robotCtrlStk[0],
               ROBOT_CTRL_STK_SIZE / HIGH_WATER_LIMIT,
               ROBOT_CTRL_STK_SIZE,
               0,
               0,
               (void *)0,
               0,
               &osErr);
  
  assert(osErr == OS_ERR_NONE);
}

/*--------------- R o b o t C t r l T a s k ---------------
Control a robot by generating step commands
*/
void RobotCtrlTask(void *data){
  Buffer *payloadBfr = NULL;
  Payload *payload;
  OS_ERR osErr;
  OS_MSG_SIZE msgSize;
  
  CPU_INT08U *p_id = data;
  CPU_INT08U id = *p_id;
  
  for(;;){
    
//    OSSemPend(&messageWaiting[id - FIRST_ROBOT], 0, OS_OPT_PEND_BLOCKING, NULL, &osErr);
//    assert(osErr == OS_ERR_NONE);
    
    if(payloadBfr == NULL){
//      payloadBfr = OSQPend(&robotCtrlQueue[*p_id],
      payloadBfr = OSQPend(&robotCtrlQueue[*p_id - FIRST_ROBOT],
                   0,
                   OS_OPT_PEND_BLOCKING,
                   &msgSize,
                   NULL,
                   &osErr);
    assert(osErr == OS_ERR_NONE);
    }
    
    payload = (Payload *) payloadBfr->buffer;
    
//    BfrReset(payloadBfr);
    Coord_t destination = payload->payloadData.robot.destination[0];
    Coord_t currentLocation = robots[payload->payloadData.robot.robotAddress - FIRST_ROBOT].location;
    
    switch(payload->msgType){
      case(MSG_MOVE):
        while((destination.x != currentLocation.x) &&
              (destination.y != currentLocation.y)){
          MoveRobot(payloadBfr);
          OSQPend(&robotCtrlMbox[payload->payloadData.robot.robotAddress - FIRST_ROBOT],
                   0,
                   OS_OPT_PEND_BLOCKING,
                   &msgSize,
                   NULL,
                   &osErr);
              }
        break;
    }
    
    payloadBfr = NULL;
  }
}

/*--------------- A d d R o b o t ---------------
Add a robot to the field
*/
void AddRobot(Buffer *payloadBfr){
  Payload *payload = (Payload *) payloadBfr->buffer;
  
  CPU_INT08U id = payload->payloadData.robot.robotAddress;
  Coord_t location = payload->payloadData.robot.destination[0];
  
  if(robots[id-FIRST_ROBOT].exists){ // Make sure the robot doesn't already exist
    SendError(payloadBfr, ERR_ADD_EXISTS);
  }else if((location.x > X_LIM) ||
           (location.y > Y_LIM)){ // Make sure you've been given a valid starting point
    SendError(payloadBfr, ERR_ADD_LOC);
  }else if((id < FIRST_ROBOT) || (id > FIRST_ROBOT + MAX_ROBOTS - 1)){ // and a valid id
    SendError(payloadBfr, ERR_ADD_ADDRESS);
  }else{
    for(CPU_INT08U j=0;j<MAX_ROBOTS;j++){
      // And make sure the starting point isn't already taken by someone else
      if((location.x == robots[j].location.x) && (location.y == robots[j].location.y)){
        SendError(payloadBfr, ERR_ADD_OCCUPIED);
        return;
      }
    }
    robots[id-FIRST_ROBOT].exists = TRUE;
    robots[id-FIRST_ROBOT].location = location;
    CreateRobotCtrlTask(id);
    SendAck(payloadBfr, MSG_ADD);
  }
}

/*--------------- M o v e R o b o t ---------------
Move a robot to the given coordinate
*/
void MoveRobot(Buffer *payloadBfr){
  OS_ERR osErr;
  
  Payload *payload = (Payload *) payloadBfr->buffer;
  CPU_INT08U direction = 0;
  Coord_t destination = payload->payloadData.robot.destination[0];
  Coord_t currentLocation = robots[payload->payloadData.robot.robotAddress - FIRST_ROBOT].location;
  
  BfrReset(payloadBfr);
  
//  while((destination.x != currentLocation.x) &&
//          (destination.y != currentLocation.y)){
            if(destination.x > currentLocation.x){
              if(destination.y < currentLocation.y)
                direction = NW;
              else if(destination.y == currentLocation.y)
                direction = N;
              else if(destination.y > currentLocation.y)
                direction = NE;
            }else if(destination.x == currentLocation.x){
              if(destination.y < currentLocation.y)
                direction = W;
              else if(destination.y > currentLocation.y)
                direction = E;
            }else if(destination.x < currentLocation.x){
              if(destination.y < currentLocation.y)
                direction = SW;
              else if(destination.y == currentLocation.y)
                direction = S;
              else if(destination.y > currentLocation.y)
                direction = SE;
            }
    BfrAddByte(payloadBfr, 9);
    BfrAddByte(payloadBfr, payload->payloadData.robot.robotAddress);
    BfrAddByte(payloadBfr, 2);
    BfrAddByte(payloadBfr, 0x07);
    BfrAddByte(payloadBfr, direction);
    
    BfrClose(payloadBfr);
    OSQPost(&framerQueue, payloadBfr, sizeof(Buffer), OS_OPT_POST_FIFO, &osErr);
    assert(osErr == OS_ERR_NONE);
    
    
    
    
//    SendAck(payloadBfr, MSG_ADD);
//  }
}