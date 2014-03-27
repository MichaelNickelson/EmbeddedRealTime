/*--------------- C o n s t a n t s . h ---------------

by: Michael Nickelson

PURPOSE - Header file
Define constants used throughout this project

CHANGES
04-30-2014 mn -  Initial submission
*/

/*----- c o n s t a n t s   u s e d   i n   t h i s   p r o j e c t -----*/
#ifndef CONSTANTS_H
#define CONSTANTS_H

#define CtrlCtrAddress 1
#define MyAddress 2

#define FIRST_ROBOT 3
#define MAX_ROBOTS 13

#define X_LIM 39
#define Y_LIM 18

#define HIGH_WATER_LIMIT 10

#define PayloadBfrSize 24
#define LONGEST_PATH 10
#define PREAMBLE_LENGTH 3

#define BfrLength 24

/*-----  Assign easy to read names to message ID -----*/
#define MSG_RESET 0
#define MSG_ADD 1
#define MSG_MOVE 2
#define MSG_PATH 3
#define MSG_LOOP 4
#define MSG_STOP 5

#define MSG_STEP 0x07
#define MSG_HERE_I_AM 9

#define MSG_ACK 0x0A
#define MSG_ERR 0x0B

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

/*----- t y p e d e f s   u s e d   i n   t h i s   p r o j e c t -----*/
typedef struct
{
  CPU_INT08U x;
  CPU_INT08U y;
} Coord_t;

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

#endif