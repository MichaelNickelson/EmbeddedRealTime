/*--------------- R o b o t C t r l . h ---------------

by: Michael Nickelson

PURPOSE - Header file
Manage the stepping of individual robots

CHANGES
04-30-2014 mn -  Initial submission
*/

#ifndef ROBOTCTRL_H
#define ROBOTCTRL_H

#include "Buffer.h"

extern OS_Q robotCtrlMbox[MAX_ROBOTS];
extern OS_Q robotCtrlQueue[MAX_ROBOTS];
extern OS_SEM messageWaiting[MAX_ROBOTS];

/*----- f u n c t i o n    p r o t o t y p e s -----*/
void CreateRobotCtrlTask(CPU_INT08U id);
void AddRobot(Buffer *payloadBfr);
void MoveRobot(Buffer *payloadBfr);
void FollowPath(Buffer *payloadBfr);

#endif