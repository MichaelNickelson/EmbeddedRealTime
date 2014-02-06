#include "includes.h"
#include "Error.h"
#include "Payload.h"

void DispErr(Error_t e, CPU_CHAR reply[]){
  
  switch(e){
    case(ERR_PREAMBLE_1):
      sprintf(reply, "\a*** ERROR: Bad Preamble Byte 1\n");
      break;
    case(ERR_PREAMBLE_2):
      sprintf(reply, "\a*** ERROR: Bad Preamble Byte 2\n");
      break;
    case(ERR_PREAMBLE_3):
      sprintf(reply, "\a*** ERROR: Bad Preamble Byte 3\n");
      break;
    case(ERR_CHECKSUM):
      sprintf(reply, "\a*** ERROR: Checksum error\n");
      break;
    case(ERR_LEN):
      sprintf(reply, "\a*** ERROR: Bad Packet Size\n");
      break;
    default:
      sprintf(reply, "Unkown Exception\n");
      break;
  }
}

void DispAssert(Assert_t a, CPU_CHAR reply[]){
  
  BSP_Ser_Printf("\a*** Info: ");
  
  switch(a){
    case(ASS_ADDRESS):
      BSP_Ser_Printf("Not My Address\n");
      break;
    default:
      BSP_Ser_Printf("Unknown Assertion\n");
      break;
  }
}

/* Separating preamble errors results in an extra function, but allows for 
an arbitrary number of preamble bytes. */
void PreambleError(CPU_INT08U bn){
  BSP_Ser_Printf("\a*** ERROR: ");
  BSP_Ser_Printf("Bad Preamble Byte %d\n",bn);
}