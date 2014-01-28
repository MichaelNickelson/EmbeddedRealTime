#include "includes.h"
#include "Errors.h"

void DispErr(Error_t e){
  
  BSP_Ser_Printf("\a*** ERROR: ");
  
  switch(e){
    case(ERR_CHECKSUM):
      BSP_Ser_Printf("Checksum error\n");
      break;
    case(ERR_LEN):
      BSP_Ser_Printf("Bad Packet Size\n");
      break;
    case(ERR_MESSAGE_TYPE):
      BSP_Ser_Printf("Unknown Message Type\n");
      break;
    case(ERR_UNKNOWN):
    default:
      BSP_Ser_Printf("Unkown Exception\n");
      break;
  }
}

void DispAssert(Assert_t a){
  
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