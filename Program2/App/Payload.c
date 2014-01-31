#include "Payload.h"
#include "includes.h"
#include "BfrPair.h"
#include "Buffer.h"

#define WindSpeedLength 2
#define PrecipLength 2
#define IDLength 10
#define MsgLength 160
#define PayloadBfrSize 14

#pragma pack(1)
typedef struct
{
  CPU_INT08U    payloadLen;
  CPU_INT08U    dstAddr;
  CPU_INT08U    srcAddr;
  CPU_INT08U    msgType;
  union
  {
    CPU_INT08S  temp;
    CPU_INT16U  pres;
    struct
    {
      CPU_INT08S dewPt;
      CPU_INT08U hum;
    } hum;
    struct
    {
      CPU_INT08U speed[WindSpeedLength];
      CPU_INT16U dir;
    } wind;
    CPU_INT16U  rad;
    CPU_INT32U  dateTime;
    CPU_INT08U  depth[PrecipLength];
    CPU_INT08U  id[IDLength];
  } dataPart;
} Payload;

void PayloadInit(BfrPair **payloadBfrPair, BfrPair **replyBfrPair){
  
  static BfrPair pBfrPair;
  static CPU_INT08U pBfr0Space[PayloadBfrSize];
  static CPU_INT08U pBfr1Space[PayloadBfrSize];
  BfrPairInit(&pBfrPair, pBfr0Space, pBfr1Space, PayloadBfrSize);
  *payloadBfrPair = &pBfrPair;
  
  static BfrPair rBfrPair;
  static CPU_INT08U rBfr0Space[PayloadBfrSize];
  static CPU_INT08U rBfr1Space[PayloadBfrSize];
  BfrPairInit(&pBfrPair, rBfr0Space, rBfr1Space, PayloadBfrSize);
  *replyBfrPair = &rBfrPair;

  return;
}

void PayloadTask(){
//  CPU_INT08U msgBfr[MsgLength];
//  Payload *payload;
//  
//  if(BfrPairSwappable(&payloadBfrPair))
//    BfrPairSwap(&payloadBfrPair);
//  
//  if(!GetBfrClosed(&payloadBfrPair) || PutBfrClosed(&replyBfrPair))
//    return;
//  
//  payload = (Payload *) GetBfrAddr(&payloadBfrPair);
//  
//  PutBfrReset(&replyBfrPair);
//  
//  switch(payload->msgType)
//    {
//    case TempMsg:
//      sprintf((CPU_CHAR *) msgBfr, "\n Temperature = %d",
//              payload->dataPart.temp);
//      break;
//    }
//  PutReplyMsg(&replyBfrPair, msgBfr);
//  PutReplyMsg(&replyBfrPair "\n");
//  
//  ClosePutBfr(&replyBfrPair);
//  OpenGetBfr(&payloadBfrPair);
}