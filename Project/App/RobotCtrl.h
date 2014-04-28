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

/*----- Limits of acceptable robot ids -----*/
#define FIRST_ROBOT 3
#define MAX_ROBOTS 13
/*-----  Longest path that can be sent by either pa or lp commands -----*/
#define LONGEST_PATH 10

extern OS_Q robotCtrlMbox[MAX_ROBOTS];
extern OS_Q robotCtrlQueue[MAX_ROBOTS];

/*----- t y p e d e f s -----*/
typedef struct
{
  CPU_INT08U x;
  CPU_INT08U y;
} Coord_t;

// Also used by RobotManager.c
#pragma pack(1)
typedef struct
{
  CPU_INT08U    payloadLen;
  CPU_INT08U    dstAddr;
  CPU_INT08U    srcAddr;
  CPU_INT08U    msgType;
  union
  {
    struct
    {
      CPU_INT08U  robotAddress;
      Coord_t destination[LONGEST_PATH];
    } robot;
    CPU_INT08U  direction;
    CPU_INT08U  ackType;
    CPU_INT08U  errorCode;
    Coord_t hereIAm;
  } payloadData;
} Payload;

/*----- f u n c t i o n    p r o t o t y p e s -----*/
void CreateRobotCtrlTask(CPU_INT08U id);
void AddRobot(Buffer *payloadBfr);
void ValidateCommand(Buffer *payloadBfr);
void StopRobot(Buffer *payloadBfr);

#endif