#include "Payload.h"
#include "includes.h"
#include "BfrPair.h"
#include "Buffer.h"
#include "Error.h"
#include "string.h"

#define LowNibble 0xF
#define Nibble 4

#define LowByte 0               /* For multibyte BCD message */
#define HighByte 1              /* For multibyte BCD message */

#define WindSpeedLength 2
#define PrecipLength 2
#define IDLength 10
#define MsgLength 160
#define PayloadBfrSize 14
#define ReplyBfrSize 80

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

void ParseTemp(Payload *payload, CPU_CHAR reply[]);
void ParsePressure(Payload *payload, CPU_CHAR reply[]);
void ParseHumidity(Payload *payload, CPU_CHAR reply[]);
void ParseWind(Payload *payload, CPU_CHAR reply[]);
void ParseRadiation(Payload *payload, CPU_CHAR reply[]);
void ParseTimeStamp(Payload *payload, CPU_CHAR reply[]);
void ParsePrecip(Payload *payload, CPU_CHAR reply[]);
void ParseID(Payload *payload, CPU_CHAR reply[]);
CPU_BOOLEAN SendReply(CPU_CHAR reply[]);

static BfrPair payloadBfrPair;
static CPU_INT08U pBfr0Space[PayloadBfrSize];
static CPU_INT08U pBfr1Space[PayloadBfrSize];

static BfrPair replyBfrPair;
static CPU_INT08U rBfr0Space[ReplyBfrSize];
static CPU_INT08U rBfr1Space[ReplyBfrSize];

void PayloadInit(BfrPair **pBfrPair, BfrPair **rBfrPair){
  /* Modifying payloadBfrPair and replyBfrPair directly seems to cause problems
     so placeholder variables pBfrPair and rBfrPair are created then mapped
     onto payloadBfrPair and replyBfrPair */
  BfrPairInit(&payloadBfrPair, pBfr0Space, pBfr1Space, PayloadBfrSize);
  BfrPairInit(&replyBfrPair, rBfr0Space, rBfr1Space, ReplyBfrSize);
  *pBfrPair = &payloadBfrPair;
  *rBfrPair = &replyBfrPair;
  
  return;
}

void PayloadTask(){
  CPU_BOOLEAN done = FALSE;
  static PayloadState pState = P;
  const CPU_INT08U MyAddress = 1; // Receiver address
  static CPU_CHAR reply[ReplyBfrSize];
  Payload *payload;
  
  // Assign easy to read names to message ID
  enum {MSG_TEMP=1, MSG_PRESSURE=2, MSG_HUMIDITY=3, MSG_WIND=4,
        MSG_RADIATION=5, MSG_TIMESTAMP=6, MSG_PRECIPITATION=7, MSG_SENSORID=8};
  
  if(pState == P){
  if(GetBfrClosed(&payloadBfrPair)&!PutBfrClosed(&replyBfrPair)){
    
      payload = (Payload *) GetBfrAddr(&payloadBfrPair);
      if(payload->payloadLen <= 0){
        DispErr((Error_t) payload->payloadLen, reply);
        pState = R;
      }else{
        if(payload->dstAddr == MyAddress){
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
        }else{
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
      done = SendReply(reply);
      if(done)
        pState = P;
    }
}

/* Parse and print each message in its own function */
void ParseTemp(Payload *payload, CPU_CHAR reply[]){
  sprintf(reply, "\nSOURCE NODE %d: TEMPERATURE MESSAGE\n  Temperature = %d\n\0",
          payload->srcAddr,
          
          payload->dataPart.temp);
}

void ParsePressure(Payload *payload, CPU_CHAR reply[]){
  sprintf(reply, "\nSOURCE NODE %d: BAROMETRIC PRESSURE MESSAGE\n  Pressure = %d\n\0",
          payload->srcAddr,
          
          Reverse2Bytes(payload->dataPart.pres));
}

void ParseHumidity(Payload *payload, CPU_CHAR reply[]){
  sprintf(reply, "\nSOURCE NODE %d: HUMIDITY MESSAGE\n  Dew Point = %d Humidity = %u\n\0",
          payload->srcAddr,
          
          payload->dataPart.hum.dewPt,payload->dataPart.hum.hum);
}

void ParseWind(Payload *payload, CPU_CHAR reply[]){
  sprintf(reply, "\nSOURCE NODE %d: WIND MESSAGE\n  Speed = %d%d%d.%d Wind Direction = %d\n\0",
          payload->srcAddr,
         
          (payload->dataPart.wind.speed[LowByte] >> Nibble),
          (payload->dataPart.wind.speed[LowByte] & LowNibble),
          (payload->dataPart.wind.speed[HighByte] >> Nibble),
          (payload->dataPart.wind.speed[HighByte] & LowNibble),
         
          Reverse2Bytes(payload->dataPart.wind.dir));
}

void ParseRadiation(Payload *payload, CPU_CHAR reply[]){
  sprintf(reply, "\nSOURCE NODE %d: SOLAR RADIATION MESSAGE\n  Solar Radiation Intensity = %u\n\0",
          payload->srcAddr,
         
          Reverse2Bytes(payload->dataPart.rad));
}

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

void ParsePrecip(Payload *payload, CPU_CHAR reply[]){
  sprintf(reply, "\nSOURCE NODE %d: PRECIPITATION MESSAGE\n  Precipitation Depth = %d%d.%d%d\n\0",
          payload->srcAddr,
          
          (payload->dataPart.wind.speed[LowByte] >> Nibble),
          (payload->dataPart.wind.speed[LowByte] & LowNibble),
          (payload->dataPart.wind.speed[HighByte] >> Nibble),
          (payload->dataPart.wind.speed[HighByte] & LowNibble));
}

void ParseID(Payload *payload, CPU_CHAR reply[]){
  sprintf(reply, "\nSOURCE NODE %d: SENSOR ID MESSAGE\n  Node ID = %s\n\0",
          payload->srcAddr,
          payload->dataPart.id);
}

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