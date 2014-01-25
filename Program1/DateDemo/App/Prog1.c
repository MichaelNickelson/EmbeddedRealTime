/*--------------- P r o g 1 . c ---------------*/

/*
by:	Michael Nickelson

PURPOSE
Read in and parse sensor data over a simulated RF link.

*/

#define MYADDRESS 1
#define WINDSPEED 2
#define PRECIP 2
#define IDLen 10

/* Include Micrium and STM headers. */
#include "includes.h"

/* Include Date module header. */
#include "PktParser.h"

/*----- c o n s t a n t    d e f i n i t i o n s -----*/

#define BaudRate 9600           /* RS232 Port Baud Rate */
  
/*----- f u n c t i o n    p r o t o t y p e s -----*/

CPU_INT32S AppMain();
CPU_INT16U Reverse2Bytes(CPU_INT16U b);
CPU_INT32U Reverse4Bytes(CPU_INT32U b);

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
      CPU_INT08U speed[WINDSPEED];
      CPU_INT16U dir;
    } wind;
    CPU_INT16U  rad;
    CPU_INT32U  dateTime;
    CPU_INT08U  depth[PRECIP];
    CPU_INT08U  id[IDLen];
  } dataPart;
} Payload;

CPU_INT32S AppMain()
{
//    Date	date1;		/* -- First date */
//    Date	date2;		/* -- Second date */
//    CPU_INT16U	dayNum1;	/* -- Day of year for first date */
//    CPU_INT16U	dayNum2;	/* -- Day of year for second date */
  
Payload payload;
CPU_INT32U packDate;

//while(TRUE){

while(&payload){
  ParsePkt(&payload);
  
  if(payload.dstAddr == MYADDRESS){
    switch (payload.msgType){
      case(1):
        BSP_Ser_Printf("SOURCE NODE %d: ",payload.srcAddr);
        BSP_Ser_Printf("TEMPERATURE MESSAGE\n");
        BSP_Ser_Printf("  Temperature = %d\n\n", payload.dataPart.temp);
        break;
      case(2):
        BSP_Ser_Printf("SOURCE NODE %d: ",payload.srcAddr);
        BSP_Ser_Printf("BAROMETRIC PRESSURE MESSAGE\n");
        BSP_Ser_Printf("  Pressure = %u\n\n", Reverse2Bytes(payload.dataPart.pres));
        break;
      case(3):
        BSP_Ser_Printf("SOURCE NODE %d: ",payload.srcAddr);
        BSP_Ser_Printf("HUMIDITY MESSAGE\n");
        BSP_Ser_Printf("  Dew Point = %d ", payload.dataPart.hum.dewPt);
        BSP_Ser_Printf("Humidity = %u\n\n", payload.dataPart.hum.hum);
        break;
      case(4):
        BSP_Ser_Printf("SOURCE NODE %d: ",payload.srcAddr);
        BSP_Ser_Printf("WIND MESSAGE\n");
        BSP_Ser_Printf("  Speed %u",(payload.dataPart.wind.speed[0] >> 4));
        BSP_Ser_Printf("%u",(payload.dataPart.wind.speed[0] & 0x0F));
        BSP_Ser_Printf("%u",(payload.dataPart.wind.speed[1] >> 4));
        BSP_Ser_Printf(".");
        BSP_Ser_Printf("%u ",(payload.dataPart.wind.speed[1] & 0x0F));
        BSP_Ser_Printf("Wind Direction = %d\n\n",Reverse2Bytes(payload.dataPart.wind.dir));
        break;
      case(5):
        BSP_Ser_Printf("SOURCE NODE %d: ",payload.srcAddr);
        BSP_Ser_Printf("SOLAR RADIATION MESSAGE\n");
        BSP_Ser_Printf("  Solar Radiation Intensity = %u\n\n", 
                       Reverse2Bytes(payload.dataPart.rad));
        break;
      case(6):
        BSP_Ser_Printf("SOURCE NODE %d: ",payload.srcAddr);
        BSP_Ser_Printf("DATE/TIME STAMP MESSAGE\n");
        packDate = Reverse4Bytes(payload.dataPart.dateTime);
        BSP_Ser_Printf("  Time Stamp = %d/%d/%d %d:%d\n\n",
                       packDate>>5 & 0xF,
                       packDate & 0x1F,
                       packDate>>9 & 0xFFF,
                       packDate>>27,
                       packDate>>21 & 0x3F);
        break;
      case(7):
        BSP_Ser_Printf("SOURCE NODE %d: ",payload.srcAddr);
        BSP_Ser_Printf("PRECIPITATION MESSAGE\n");
        BSP_Ser_Printf("  Precipitation Depth = %d",(payload.dataPart.wind.speed[0] >> 4));
        BSP_Ser_Printf("%d",(payload.dataPart.wind.speed[0] & 0x0F));
        BSP_Ser_Printf(".");
        BSP_Ser_Printf("%d",(payload.dataPart.wind.speed[1] >> 4));
        BSP_Ser_Printf("%d\n\n",(payload.dataPart.wind.speed[1] & 0x0F));
        break;
      case(8):
        BSP_Ser_Printf("SOURCE NODE %d: ",payload.srcAddr);
        BSP_Ser_Printf("SENSOR ID MESSAGE\n");
        BSP_Ser_Printf("  Node ID = ");
        for(int i=0;i<payload.payloadLen-4;i++)
          BSP_Ser_Printf("%c",payload.dataPart.id[i]);
        BSP_Ser_Printf("\n\n");
        break;
      default:
        BSP_Ser_Printf("*** ERROR: Unknown Message Type\n\n");
    }
  }else{
    BSP_Ser_Printf("*** INFO: Not My Address\n\n");
  }
}
    return 0;
}

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