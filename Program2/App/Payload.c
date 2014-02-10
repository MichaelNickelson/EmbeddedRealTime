/*--------------- P a y l o a d . c ---------------

by: Michael Nickelson

PURPOSE
Parse payload and handle response

CHANGES
02/19/2014 mn - Initial submission
*/

#include "Payload.h"
#include "includes.h"
#include "BfrPair.h"
#include "Buffer.h"
#include "Error.h"
#include "string.h"

/*----- c o n s t a n t    d e f i n i t i o n s -----*/
#define LowNibble 0xF
#define Nibble 4

#define LowByte 0               /* For multibyte BCD message */
#define HighByte 1              /* For multibyte BCD message */

#define MyAddress 1

#define WindSpeedLength 2
#define PrecipLength 2
#define IDLength 10
#define MsgLength 160
#define PayloadBfrSize 14
#define ReplyBfrSize 80

/*-----  Assign easy to read names to message ID -----*/
#define MSG_TEMP 1
#define MSG_PRESSURE 2
#define MSG_HUMIDITY 3
#define MSG_WIND 4
#define MSG_RADIATION 5
#define MSG_TIMESTAMP 6
#define MSG_PRECIPITATION 7
#define MSG_SENSORID 8

/*----- t y p e d e f s   u s e d   i n   p a y l o a d   m o d u l e -----*/
#pragma pack(1)
typedef struct
{
  CPU_INT08S    payloadLen;
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

typedef enum { P, R } PayloadState;

/*----- f u n c t i o n    p r o t o t y p e s -----*/
void ParseTemp(Payload *payload, CPU_CHAR reply[]);
void ParsePressure(Payload *payload, CPU_CHAR reply[]);
void ParseHumidity(Payload *payload, CPU_CHAR reply[]);
void ParseWind(Payload *payload, CPU_CHAR reply[]);
void ParseRadiation(Payload *payload, CPU_CHAR reply[]);
void ParseTimeStamp(Payload *payload, CPU_CHAR reply[]);
void ParsePrecip(Payload *payload, CPU_CHAR reply[]);
void ParseID(Payload *payload, CPU_CHAR reply[]);
CPU_BOOLEAN SendReply(CPU_CHAR reply[]);


/*----- Declare i/o buffer pairs -----*/
static BfrPair payloadBfrPair;
static CPU_INT08U pBfr0Space[PayloadBfrSize];
static CPU_INT08U pBfr1Space[PayloadBfrSize];

static BfrPair replyBfrPair;
static CPU_INT08U rBfr0Space[ReplyBfrSize];
static CPU_INT08U rBfr1Space[ReplyBfrSize];


/*--------------- P a y l o a d I n i t ---------------
Initialize payload and reply buffer pairs.
*/
void PayloadInit(BfrPair **pBfrPair, BfrPair **rBfrPair){
  /* Modifying payloadBfrPair and replyBfrPair directly seems to cause problems
     so placeholder variables pBfrPair and rBfrPair are created then mapped
     onto payloadBfrPair and replyBfrPair */
  BfrPairInit(&payloadBfrPair, pBfr0Space, pBfr1Space, PayloadBfrSize);
  BfrPairInit(&replyBfrPair, rBfr0Space, rBfr1Space, ReplyBfrSize);
  *pBfrPair = &payloadBfrPair;
  *rBfrPair = &replyBfrPair;
}

/*--------------- P a y l o a d T a s k ---------------
Get a payload from payloadBfrPair and generate a reply based on message type 
then forward it to the reply buffer
*/
void PayloadTask(){
  CPU_BOOLEAN replyDone = FALSE;
  static PayloadState pState = P;
  static CPU_CHAR reply[ReplyBfrSize];
  Payload *payload;
  
  if(pState == P){ // If no reply is being sent check payload data ready conditions
    if(GetBfrClosed(&payloadBfrPair)&!PutBfrClosed(&replyBfrPair)){
      payload = (Payload *) GetBfrAddr(&payloadBfrPair);
      if(payload->payloadLen <= 0){  // Check for error cases
        DispErr((Error_t) payload->payloadLen, reply);
        pState = R;
      }else{
        if(payload->dstAddr == MyAddress){ // If message is to me, parse a response
          switch(payload->msgType){
            case(MSG_TEMP):
              ParseTemp(payload, reply);
              break;
            case(MSG_PRESSURE):
              ParsePressure(payload, reply);
              break;
            case(MSG_HUMIDITY):
              ParseHumidity(payload, reply);
              break;
            case(MSG_WIND):
              ParseWind(payload, reply);
              break;
            case(MSG_RADIATION):
              ParseRadiation(payload, reply);
              break;
            case(MSG_TIMESTAMP):
              ParseTimeStamp(payload, reply);
              break;
            case(MSG_PRECIPITATION):
              ParsePrecip(payload, reply);
              break;
            case(MSG_SENSORID):
              ParseID(payload, reply);
              break;
            default:  // Handle unknown message types
              DispErr((Error_t) payload->msgType, reply);
              break;
          }
          pState = R;
        }else{ // Display an info message if another host is the target
          DispAssert(ASS_ADDRESS, reply);
          if(BfrPairSwappable(&replyBfrPair))
            BfrPairSwap(&replyBfrPair);
          pState = R;
        }
      }
      OpenGetBfr(&payloadBfrPair);
      if(BfrPairSwappable(&payloadBfrPair))
            BfrPairSwap(&payloadBfrPair);
    }
  }else{
      replyDone = SendReply(reply);
      if(replyDone)
        pState = P;
    }
}

/* Parse and print each message in its own function */

/*--------------- P a r s e T e m p ---------------
Generate a temperate message
*/
void ParseTemp(Payload *payload, CPU_CHAR reply[]){
  sprintf(reply, "\nSOURCE NODE %d: TEMPERATURE MESSAGE\n  Temperature = %d\n\0",
          payload->srcAddr,
          
          payload->dataPart.temp);
}

/*--------------- P a r s e P r e s s u r e ---------------
Generate a pressure message
*/
void ParsePressure(Payload *payload, CPU_CHAR reply[]){
  sprintf(reply, "\nSOURCE NODE %d: BAROMETRIC PRESSURE MESSAGE\n  Pressure = %d\n\0",
          payload->srcAddr,
          
          Reverse2Bytes(payload->dataPart.pres));
}

/*--------------- P a r s e H u m i d i t y ---------------
Generate a humidity message
*/
void ParseHumidity(Payload *payload, CPU_CHAR reply[]){
  sprintf(reply, "\nSOURCE NODE %d: HUMIDITY MESSAGE\n  Dew Point = %d Humidity = %u\n\0",
          payload->srcAddr,
          
          payload->dataPart.hum.dewPt,payload->dataPart.hum.hum);
}

/*--------------- P a r s e W i n d ---------------
Generate a wind message
*/
void ParseWind(Payload *payload, CPU_CHAR reply[]){
  sprintf(reply, "\nSOURCE NODE %d: WIND MESSAGE\n  Speed = %d%d%d.%d Wind Direction = %d\n\0",
          payload->srcAddr,
         
          (payload->dataPart.wind.speed[LowByte] >> Nibble),
          (payload->dataPart.wind.speed[LowByte] & LowNibble),
          (payload->dataPart.wind.speed[HighByte] >> Nibble),
          (payload->dataPart.wind.speed[HighByte] & LowNibble),
         
          Reverse2Bytes(payload->dataPart.wind.dir));
}

/*--------------- P a r s e R a d i a t i o n ---------------
Generate a radiation message
*/
void ParseRadiation(Payload *payload, CPU_CHAR reply[]){
  sprintf(reply, "\nSOURCE NODE %d: SOLAR RADIATION MESSAGE\n  Solar Radiation Intensity = %u\n\0",
          payload->srcAddr,
         
          Reverse2Bytes(payload->dataPart.rad));
}

/*--------------- P a r s e T i m e S t a m p ---------------
Generate a time/date message
*/
void ParseTimeStamp(Payload *payload, CPU_CHAR reply[]){
  /* Starting bit and bitmask for each component of packed date/time message */
  const CPU_INT08U DayPosition    = 0;
  const CPU_INT08U MonthPosition  = 5;
  const CPU_INT08U YearPosition   = 9;
  const CPU_INT08U MinutePosition = 21;
  const CPU_INT08U HourPosition   = 27;
  
  const CPU_INT08U DayMask        = 0x1F;
  const CPU_INT08U MonthMask      = 0xF;
  const CPU_INT16U YearMask       = 0xFFF;
  const CPU_INT08U MinuteMask     = 0x3F;
  const CPU_INT08U HourMask       = 0x1F;
  
  CPU_INT32U packDate = Reverse4Bytes(payload->dataPart.dateTime);
  
  sprintf(reply, "\nSOURCE NODE %d: DATE/TIME STAMP MESSAGE\n  Time Stamp = %d/%d/%d %d:%d\n\0",
          payload->srcAddr,
          
          packDate>>MonthPosition  & MonthMask,
          packDate>>DayPosition    & DayMask,
          packDate>>YearPosition   & YearMask,
          packDate>>HourPosition   & HourMask,
          packDate>>MinutePosition & MinuteMask);
}

/*--------------- P a r s e P r e c i p ---------------
Generate a precipitation message
*/
void ParsePrecip(Payload *payload, CPU_CHAR reply[]){
  sprintf(reply, "\nSOURCE NODE %d: PRECIPITATION MESSAGE\n  Precipitation Depth = %d%d.%d%d\n\0",
          payload->srcAddr,
          
          (payload->dataPart.wind.speed[LowByte] >> Nibble),
          (payload->dataPart.wind.speed[LowByte] & LowNibble),
          (payload->dataPart.wind.speed[HighByte] >> Nibble),
          (payload->dataPart.wind.speed[HighByte] & LowNibble));
}

/*--------------- P a r s e I D ---------------
Generate an ID message
*/
void ParseID(Payload *payload, CPU_CHAR reply[]){
  sprintf(reply, "\nSOURCE NODE %d: SENSOR ID MESSAGE\n  Node ID = %s\n\0",
          payload->srcAddr,
          payload->dataPart.id);
}

/*--------------- S e n d R e p l y ---------------
Send the reply message to the replyBfrPair one byte at a time.
If the entire message has been sent, return true
*/
CPU_BOOLEAN SendReply(CPU_CHAR reply[]){
  static CPU_INT08S j = 0;
  CPU_BOOLEAN retVal = FALSE;

  if(j<strlen(reply)){
    PutBfrAddByte(&replyBfrPair, reply[j]);
    j++;
  }else{
    ClosePutBfr(&replyBfrPair);
    if(BfrPairSwappable(&replyBfrPair))
      BfrPairSwap(&replyBfrPair);
    j = 0;
    retVal = TRUE;
  }
  
  return retVal;
}

// Byte reversal functions for 1 and 2 word ints
CPU_INT16U Reverse2Bytes(CPU_INT16U b){
  b = (b<<8) | (b>>8);
  return b;
}

CPU_INT32U Reverse4Bytes(CPU_INT32U b){
  b = (b<<24) | ((b<<8)&0xFF0000) | ((b>>8)&0xFF00) | (b>>24);
  return b;
}