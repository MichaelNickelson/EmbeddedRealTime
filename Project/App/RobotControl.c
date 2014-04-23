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
#include "Error.h"
#include "Framer.h"
#include "MemMgr.h"
#include "RobotManager.h"

#define RobotCtrlPrio 4
#define ROBOT_CTRL_STK_SIZE 128
#define PAYLOAD_OVERHEAD 5
#define DIMENSIONS 2
#define DIRECTIONS 8
#define SUSPEND_TIMEOUT 2000
#define MISSED_STEPS 6

typedef struct
{
  CPU_INT08U id;
  Coord_t location;
  Coord_t nextLocation;
  CPU_BOOLEAN exists;
} Robot_t;

/*-----  Directions used for step packets -----*/
#define N 1
#define NE 2
#define E 3
#define SE 4
#define S 5
#define SW 6
#define W 7
#define NW 8
#define NoStep 0

/*----- Dimensions of the field being used -----*/
#define X_LIM 39
#define Y_LIM 18

/*----- l o c a l   f u n c t i o n    p r o t o t y p e s -----*/
void RobotCtrlTask(void *data);
void StepRobot(Buffer *stepBuffer, Coord_t destination, Robot_t *robot);
CPU_INT08U CheckSafety(CPU_INT08S direction, Robot_t *robot);

/*----- G l o b a l   V a r i a b l e s -----*/
// Task TCB and stack
static OS_TCB robotCtrlTCB[MAX_ROBOTS];
static CPU_STK robotCtrlStk[MAX_ROBOTS][ROBOT_CTRL_STK_SIZE];
OS_Q robotCtrlMbox[MAX_ROBOTS];
OS_Q robotCtrlQueue[MAX_ROBOTS];

OS_SEM messageWaiting[MAX_ROBOTS];

// Array of robots for collision and existence detection.
Robot_t robots[MAX_ROBOTS];

/*--------------- C r e a t e R o b o t C t r l T a s k ---------------
Create/Initialize robot controller task
*/
void CreateRobotCtrlTask(CPU_INT08U id){
  OS_ERR osErr;
  
  OSQCreate(&robotCtrlMbox[id-FIRST_ROBOT], "Robot Mailbox", 1, &osErr);
  assert(osErr == OS_ERR_NONE);
  
  OSQCreate(&robotCtrlQueue[id-FIRST_ROBOT], "Robot Queue", PoolSize, &osErr);
  assert(osErr == OS_ERR_NONE);
  
  // Create the payload task
  OSTaskCreate(&robotCtrlTCB[id-FIRST_ROBOT],
               "Robot controller task",
               &RobotCtrlTask,
               &robots[id - FIRST_ROBOT],
//               &id,
               RobotCtrlPrio,
               &robotCtrlStk[id - FIRST_ROBOT][0],
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
This function will make a path consisting of each point sent from CtrlCtr and
generate a series of step commands to move the robot from one point to the next.
Once a step diretion is determined it's safety is checked and adjusted if necessary.
After MISSED_STEPS unsafe steps in a row the current waypoint is skipped. If a
stop looping command is received the loop is broken and the robot stops whereever
it happens to be at the time.
*/
void RobotCtrlTask(void *data){
  OS_ERR osErr;
  Buffer *payloadBfr = NULL;
  CPU_INT08U numPoints = 0; // Number of destinations in the current path
  OS_MSG_SIZE msgSize;
  CPU_INT08U oDist = 0; // Distance to the current destination
  CPU_INT08U unSafe = 0; // Number of unsafe steps the robot has taken of this leg
  Coord_t pathPoints[LONGEST_PATH]; // The path that the robot will follow
  CPU_BOOLEAN looping = FALSE; // Is the robot following a loop
  CPU_BOOLEAN breakLoop = FALSE;
  Robot_t *rob = (Robot_t *) data;
  
  for(;;){
    payloadBfr = OSQPend(&robotCtrlQueue[rob->id - FIRST_ROBOT], 0, OS_OPT_PEND_BLOCKING, &msgSize, NULL, &osErr);
    assert(osErr == OS_ERR_NONE);
    Payload *payload = (Payload *) payloadBfr->buffer;
    looping = payload->msgType == MSG_LOOP;
    // Store the path in a safe variable
    numPoints = (payload->payloadLen - PAYLOAD_OVERHEAD)/DIMENSIONS;
    for(CPU_INT08U j=0;j<numPoints;j++)
      pathPoints[j] = payload->payloadData.robot.destination[j];
    do{ // As long as looping is true, stay in this loop
      for(CPU_INT08U j = 0;j<numPoints;j++){ // Visit each destination
        unSafe = 0; // Since it's just started travelling, no unsafe steps have been taken.
        do{ // Take at least one step, so that even NoStep commands will generate a HereIAm
          oDist = abs(rob->location.x - pathPoints[j].x) + abs(rob->location.y - pathPoints[j].y);
          StepRobot(payloadBfr, pathPoints[j], &robots[rob->id - FIRST_ROBOT]);
          while((rob->location.x != rob->nextLocation.x) || (rob->location.y != rob->nextLocation.y)){
            payloadBfr = OSQPend(&robotCtrlMbox[rob->id - FIRST_ROBOT], SUSPEND_TIMEOUT, OS_OPT_PEND_BLOCKING, &msgSize, NULL, &osErr);
            assert((osErr == OS_ERR_NONE) || (osErr == OS_ERR_TIMEOUT));
            if(OS_ERR_TIMEOUT == osErr){
              payloadBfr = Allocate(); // Pass a valid puffer to Steprobot
              StepRobot(payloadBfr, pathPoints[j], &robots[rob->id - FIRST_ROBOT]);
            }else if(OS_ERR_NONE == osErr){
              payload = (Payload *) payloadBfr->buffer;
              if(payload->msgType == MSG_STOP){
                breakLoop = TRUE;
              }else if(MSG_HERE_I_AM == payload->msgType){
                payload = (Payload *) payloadBfr->buffer;
              rob->location = payload->payloadData.hereIAm;
              }
            }
          }
          if(breakLoop){
            looping = FALSE;
            breakLoop = FALSE;
            break;
          }
          // If the robot isn't closer to the destination, that step was unsafe
          unSafe += (oDist > (abs(rob->location.x - pathPoints[j].x) + abs(rob->location.y - pathPoints[j].y))) ? 0 : 1;
          // After cumulative MISSED_STEPS that aren't closer to the destination, move on.
          if(unSafe > MISSED_STEPS)
            break;
        }while((rob->location.x != pathPoints[j].x) ||
               (rob->location.y != pathPoints[j].y));
        if(!looping)
          break;
      }
    }while(looping);
  }
}

/*--------------- A d d R o b o t ---------------
Add a robot to the field
*/
void AddRobot(Buffer *payloadBfr){
  Payload *payload = (Payload *) payloadBfr->buffer;
  
  CPU_INT08U id = payload->payloadData.robot.robotAddress;
  Coord_t location = payload->payloadData.robot.destination[0];
  
  if((id < FIRST_ROBOT) || (id > FIRST_ROBOT + MAX_ROBOTS - 1)){ // Is this a valid id?
    SendError(ERR_ADD_ADDRESS);
  }else if((location.x > X_LIM) ||
           (location.y > Y_LIM)){ // Make sure you've been given a valid starting point
    SendError(ERR_ADD_LOC);
  }else if(robots[id-FIRST_ROBOT].exists){ // Make sure the robot doesn't already exist
    SendError(ERR_ADD_EXISTS);
  }else{
    for(CPU_INT08U j=0;j<MAX_ROBOTS;j++){
      // And make sure the starting point isn't already taken by someone else
      if((location.x == robots[j].location.x) &&
         (location.y == robots[j].location.y) &&
          robots[j].exists){
        SendError(ERR_ADD_OCCUPIED);
        return;
      }
    }
    CreateRobotCtrlTask(id);
    robots[id-FIRST_ROBOT].exists = TRUE;
    robots[id-FIRST_ROBOT].location = location;
    robots[id-FIRST_ROBOT].nextLocation = location;
    robots[id-FIRST_ROBOT].id = id;
    SendAck(MSG_ADD);
  }
  Free(payloadBfr);
}

/*--------------- V a l i d a t e C o m m a n d ---------------
Ensure that a command involving robot movement is valid
*/
void ValidateCommand(Buffer *payloadBfr){
  OS_ERR osErr;
  Payload *payload = (Payload *) payloadBfr->buffer;
  CPU_INT08U numPoints = (payload->payloadLen - PAYLOAD_OVERHEAD)/DIMENSIONS;
  
  CPU_INT08U id = payload->payloadData.robot.robotAddress;
  Coord_t destination[LONGEST_PATH];

  for(CPU_INT08U j=0;j<numPoints;j++){
    destination[j] = payload->payloadData.robot.destination[j];
  }
  
  // Make sure the address is valid
  if((id < FIRST_ROBOT) || (id > FIRST_ROBOT + MAX_ROBOTS - 1)){
    SendError((Error_t) (ERROR_MULTIPLIER*payload->msgType + ROBOT_ADDRESS));
    Free(payloadBfr);
    return;
  // Make sure the robot exists
  }else if(!robots[payload->payloadData.robot.robotAddress - FIRST_ROBOT].exists){
    SendError((Error_t) (ERROR_MULTIPLIER*payload->msgType + NON_EXISTENT_ROBOT));
    Free(payloadBfr);
    return;
  }else{
    // Make sure all the destination points are on the field
    for(CPU_INT08U j=0;j<numPoints;j++){
      if((destination[j].x > X_LIM) || (destination[j].y > Y_LIM)){
        SendError((Error_t) (ERROR_MULTIPLIER*payload->msgType + BAD_LOCATION));
        Free(payloadBfr);
        return;
      }
    }
    
    // If everything is good, post a message to the appropriate ctrlQueue and send Ack
    OSQPost(&robotCtrlQueue[(payload->payloadData.robot.robotAddress) - FIRST_ROBOT],
            payloadBfr, sizeof(Buffer), OS_OPT_POST_FIFO, &osErr);
    assert(osErr == OS_ERR_NONE);
    
    SendAck(payload->msgType);
  }
}

/*--------------- S t e p R o b o t ---------------
Determine which direction a robot should step in
*/
void StepRobot(Buffer *stepBuffer, Coord_t destination, Robot_t *robot){
  CPU_INT08S direction = NoStep;
  Coord_t currentLocation = robot->location;
  Coord_t nextLocation = robot->nextLocation;
  
  if((nextLocation.x != currentLocation.x) || nextLocation.y != currentLocation.y)
    direction = NoStep;
  else if((destination.x == currentLocation.x) && (destination.y > currentLocation.y))
     direction = N;
  else if((destination.x > currentLocation.x) && (destination.y > currentLocation.y))
    direction = NE;
  else if((destination.x > currentLocation.x) && (destination.y == currentLocation.y))
    direction = E;
  else if((destination.x > currentLocation.x) && (destination.y < currentLocation.y))
    direction = SE;
  else if((destination.x == currentLocation.x) && (destination.y < currentLocation.y))
    direction = S;
  else if((destination.x < currentLocation.x) && (destination.y < currentLocation.y))
    direction = SW;
  else if((destination.x < currentLocation.x) && (destination.y == currentLocation.y))
    direction = W;
  else if((destination.x < currentLocation.x) && (destination.y > currentLocation.y))
    direction = NW;
  
  if(direction != NoStep)
    direction = CheckSafety(direction, robot);
  
//  stepBuffer = Allocate();
  MakePayload(stepBuffer, robot->id, MSG_STEP, direction);
}

/*--------------- S t o p R o b o t ---------------
Stop a looping robot
*/
void StopRobot(Buffer *payloadBfr){
  OS_ERR osErr;
  Payload *payload = (Payload *) payloadBfr->buffer;
  CPU_INT08U id = payload->payloadData.robot.robotAddress;
  
  // Make sure the address is valid
  if((id < FIRST_ROBOT) || (id > FIRST_ROBOT + MAX_ROBOTS - 1)){
    SendError((Error_t) (ERROR_MULTIPLIER*MSG_STOP + ROBOT_ADDRESS));
    Free(payloadBfr);
    return;
  // Make sure the robot exists
  }else if(!robots[id - FIRST_ROBOT].exists){
    SendError((Error_t) (ERROR_MULTIPLIER*payload->msgType + NON_EXISTENT_ROBOT));
    Free(payloadBfr);
    return;
  }
    
  OSQPost(&robotCtrlMbox[id - FIRST_ROBOT],
          payloadBfr, sizeof(Buffer), OS_OPT_POST_FIFO, &osErr);
  assert(osErr == OS_ERR_NONE);
  
  SendAck(MSG_STOP);
}

/*--------------- C h e c k S a f e t y ---------------
Ensure that the robot moves in a safe direction
*/
CPU_INT08U CheckSafety(CPU_INT08S direction, Robot_t *robot){
  Coord_t currentLocation = robot->location;
  CPU_BOOLEAN dirSafe;
  CPU_INT08S oDir = direction;
  // The change in x,y for each possible direction
  CPU_INT08U change[DIRECTIONS+1][DIMENSIONS] = {{0,0},{0,1},{1,1},{1,0},{1,-1},{0,-1},{-1,-1},{-1,0},{-1,1}};
  CPU_INT08S newDirection = 0; // The change to direction for the next try
  Coord_t nextLocation;
  
  // Loop over all possible directions
  for(CPU_INT08U j=0;j<DIRECTIONS;j++){
    dirSafe = TRUE;
    // Set and unwrap the direction to be tested
    direction = oDir + newDirection;
    // Try to stay as close to the desired direction as possible
    // (0, 1, -1, 2, -2, 3, -3, ...)
    newDirection = newDirection > 0 ? -newDirection : -newDirection+1;
    if(direction > DIRECTIONS)
      direction -= DIRECTIONS;
    else if(direction < 1)
      direction += DIRECTIONS;
    // Figure out which point is going to be tested for safety
    nextLocation.x = currentLocation.x + change[direction][0];
    nextLocation.y = currentLocation.y + change[direction][1];
    // Make sure there's not already a robot there
    for(CPU_INT08U k=0;k<MAX_ROBOTS;k++){
      if((robots[k].exists) &&
         (robots[k].location.x == nextLocation.x) &&
         (robots[k].location.y == nextLocation.y)){
        dirSafe = FALSE;
        break;
      }
      // Make sure there's not another robot on its way to that spot
      // This is especially useful with large buffers where the hereIAm message
      // waiting to be picked up. It can add steps in highly conjested areas though.
      if((robots[k].exists) &&
         (robots[k].nextLocation.x == nextLocation.x) &&
         (robots[k].nextLocation.y == nextLocation.y)){
        dirSafe = FALSE;
        break;
      }
    }
    // Make sure it's inside the field
    if((nextLocation.x > X_LIM) || (nextLocation.y > Y_LIM))
      dirSafe = FALSE;
    
    if(dirSafe)
      break;
  }
  
  if(!dirSafe){
    direction = NoStep;
  }
  // The robot's location is set here to prevent another robot from stepping to the
  // same location before the HereIAm packet is received.
  robot->nextLocation = nextLocation;
  return direction;
}