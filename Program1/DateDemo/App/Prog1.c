/*--------------- P r o g 1 . c ---------------*/

/*
by:	Michael Nickelson

PURPOSE
Read in and parse sensor data over a simulated RF link.

*/

/*----- Parameters for this program -----*/

#define MYADDRESS 1             /* Receiver address */                

#define WINDSPEED_LENGTH 2      /* 2-byte BCD */
#define PRECIP_LENGTH 2         /* 2-byte BCD */
#define ID_LENGTH 10            /* Up to 10 ASCII characters */

#define BaudRate 9600           /* RS232 Port Baud Rate */

/* Include Micrium and STM headers. */
#include "includes.h"

/* Include module headers. */
#include "PktParser.h"
#include "Errors.h"

/*----- Definition of Payload structure -----*/
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
      CPU_INT08U speed[WINDSPEED_LENGTH];
      CPU_INT16U dir;
    } wind;
    CPU_INT16U  rad;
    CPU_INT32U  dateTime;
    CPU_INT08U  depth[PRECIP_LENGTH];
    CPU_INT08U  id[ID_LENGTH];
  } dataPart;
} Payload;
  
/*----- f u n c t i o n    p r o t o t y p e s -----*/

CPU_INT32S AppMain();

CPU_INT16U Reverse2Bytes(CPU_INT16U b); // byte reverse a single word
CPU_INT32U Reverse4Bytes(CPU_INT32U b); // byte reverse a 32-bit int

/*----- parsing function prototypes -----*/

void ParseTemp(Payload *payload);
void ParsePressure(Payload *payload);
void ParseHumidity(Payload *payload);
void ParseWind(Payload *payload);
void ParseRadiation(Payload *payload);
void ParseTimeStamp(Payload *payload);
void ParsePrecip(Payload *payload);
void ParseID(Payload *payload);

/*--------------- m a i n ( ) -----------------*/

CPU_INT32S main()
{
    CPU_INT32S	exitCode;       // Return this code on exit.
	
//  Initialize the STM32F107 eval. board.
    BSP_IntDisAll();            /* Disable all interrupts. */

    BSP_Init();                 /* Initialize BSP functions */

    BSP_Ser_Init(BaudRate);     /* Initialize the RS232 interface. */

//  Run the application.
    exitCode = AppMain();
    
    return exitCode;
}

/*--------------- A p p M a i n ( ) -----------------*/

CPU_INT32S AppMain()
{
  // Assign easy to read names to message ID
  enum {MSG_TEMP=1,
        MSG_PRESSURE=2,
        MSG_HUMIDITY=3,
        MSG_WIND=4,
        MSG_RADIATION=5,
        MSG_TIMESTAMP=6,
        MSG_PRECIPITATION=7,
        MSG_SENSORID=8};
  
  Payload payload; // Instantiate message payload

  while(&payload){
    ParsePkt(&payload);
    
    if(payload.dstAddr == MYADDRESS){   // Only parse messages to this receiver
      switch (payload.msgType){         // Call message parser for appropriate
        case(MSG_TEMP):                 // message type
          ParseTemp(&payload);
          break;
        case(MSG_PRESSURE):
          ParsePressure(&payload);
          break;
        case(MSG_HUMIDITY):
          ParseHumidity(&payload);
          break;
        case(MSG_WIND):
          ParseWind(&payload);
          break;
        case(MSG_RADIATION):
          ParseRadiation(&payload);
          break;
        case(MSG_TIMESTAMP):
          ParseTimeStamp(&payload);
          break;
        case(MSG_PRECIPITATION):
          ParsePrecip(&payload);
          break;
        case(MSG_SENSORID):
          ParseID(&payload);
          break;
        default:                        // Handle unknown message types
          DispErr(ERR_MESSAGE_TYPE);
          break;
      }
    }else{
      DispAssert(ASS_ADDRESS);          // Messages to other receivers
    }
  }
    return 0;
}

/* Parse and print each message in its own function */
void ParseTemp(Payload *payload){
  BSP_Ser_Printf("\nSOURCE NODE %d: ",payload->srcAddr);
  BSP_Ser_Printf("TEMPERATURE MESSAGE\n");
  BSP_Ser_Printf("  Temperature = %d\n", payload->dataPart.temp);
}

void ParsePressure(Payload *payload){
  BSP_Ser_Printf("\nSOURCE NODE %d: ",payload->srcAddr);
  BSP_Ser_Printf("BAROMETRIC PRESSURE MESSAGE\n");
  BSP_Ser_Printf("  Pressure = %u\n", Reverse2Bytes(payload->dataPart.pres));
}

void ParseHumidity(Payload *payload){
  BSP_Ser_Printf("\nSOURCE NODE %d: ",payload->srcAddr);
  BSP_Ser_Printf("HUMIDITY MESSAGE\n");
  BSP_Ser_Printf("  Dew Point = %d ", payload->dataPart.hum.dewPt);
  BSP_Ser_Printf("Humidity = %u\n", payload->dataPart.hum.hum);
}

void ParseWind(Payload *payload){
  BSP_Ser_Printf("\nSOURCE NODE %d: ",payload->srcAddr);
  BSP_Ser_Printf("WIND MESSAGE\n");
  BSP_Ser_Printf("  Speed = %d%d%d.%d ",(payload->dataPart.wind.speed[0] >> 4),
                                        (payload->dataPart.wind.speed[0] & 0x0F),
                                        (payload->dataPart.wind.speed[1] >> 4),
                                        (payload->dataPart.wind.speed[1] & 0x0F));
  BSP_Ser_Printf("Wind Direction = %d\n",Reverse2Bytes(payload->dataPart.wind.dir));
}

void ParseRadiation(Payload *payload){
  BSP_Ser_Printf("\nSOURCE NODE %d: ",payload->srcAddr);
  BSP_Ser_Printf("SOLAR RADIATION MESSAGE\n");
  BSP_Ser_Printf("  Solar Radiation Intensity = %u\n", 
                 Reverse2Bytes(payload->dataPart.rad));
}

void ParseTimeStamp(Payload *payload){
  CPU_INT32U packDate;
  
  BSP_Ser_Printf("\nSOURCE NODE %d: ",payload->srcAddr);
  BSP_Ser_Printf("DATE/TIME STAMP MESSAGE\n");
  packDate = Reverse4Bytes(payload->dataPart.dateTime);
  BSP_Ser_Printf("  Time Stamp = %d/%d/%d %d:%d\n",
                 packDate>>5 & 0xF,
                 packDate & 0x1F,
                 packDate>>9 & 0xFFF,
                 packDate>>27,
                 packDate>>21 & 0x3F);
}

void ParsePrecip(Payload *payload){
  BSP_Ser_Printf("\nSOURCE NODE %d: ",payload->srcAddr);
  BSP_Ser_Printf("PRECIPITATION MESSAGE\n");
  BSP_Ser_Printf("  Precipitation Depth = %d%d.%d%d\n",
                 (payload->dataPart.wind.speed[0] >> 4),
                 (payload->dataPart.wind.speed[0] & 0x0F),
                 (payload->dataPart.wind.speed[1] >> 4),
                 (payload->dataPart.wind.speed[1] & 0x0F));
}

void ParseID(Payload *payload){
  BSP_Ser_Printf("\nSOURCE NODE %d: ",payload->srcAddr);
  BSP_Ser_Printf("SENSOR ID MESSAGE\n");
  BSP_Ser_Printf("  Node ID = ");
  for(int i=0;i<payload->payloadLen-4;i++)
    BSP_Ser_Printf("%c",payload->dataPart.id[i]);
  BSP_Ser_Printf("\n");
}

// Byte reversal functions for 1 and 2 word ints
CPU_INT16U Reverse2Bytes(CPU_INT16U b){
  b = (b<<8)|
      (b>>8);
  return b;
}

CPU_INT32U Reverse4Bytes(CPU_INT32U b){
  b = (b<<24)|
      ((b<<8)&0x00FF0000)|
      ((b>>8)&0x0000FF00)|
      (b>>24);
  return b;
}