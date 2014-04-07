/*--------------- C o n s t a n t s . h ---------------

by: Michael Nickelson

PURPOSE - Header file
Define constants and structures used throughout this project

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

typedef struct
{
  CPU_INT08U bfrSpace[PayloadBfrSize];
} BufferSpace_t;

#endif