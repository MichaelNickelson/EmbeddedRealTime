#include "includes.h"
#include "Errors.h"

void DispErr(Error_t e){
  
  BSP_Ser_Printf("\a*** ERROR: ");
  
  switch(e){
    case(ERR_PRE1):
      BSP_Ser_Printf("Bad Preamble Byte 1\n");
      break;
    case(ERR_PRE2):
      BSP_Ser_Printf("Bad Preamble Byte 2\n");
      break;
    case(ERR_PRE3):
      BSP_Ser_Printf("Bad Preamble Byte 3\n");
      break;
    case(ERR_CHECKSUM):
      BSP_Ser_Printf("Bad Checksum\n");
      break;
    case(ERR_LEN):
      BSP_Ser_Printf("Bad Packet Size\n");
      break;
    case(ERR_MESSAGE_TYPE):
      BSP_Ser_Printf("Unknown Message Type\n");
      break;
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