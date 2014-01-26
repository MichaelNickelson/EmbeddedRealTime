#include "includes.h"
#include "Errors.h"

void DispErr(Error_t err){
  
  BSP_Ser_Printf("\a*** ERROR: ");
  
  switch(err){
    case(ERR_PRE1):
      BSP_Ser_Printf("Bad Preamble Byte 1");
      break;
    case(ERR_PRE2):
      BSP_Ser_Printf("Bad Preamble Byte 2");
      break;
    case(ERR_PRE3):
      BSP_Ser_Printf("Bad Preamble Byte 3");
      break;
    case(ERR_CHECKSUM):
      BSP_Ser_Printf("Bad Checksum");
      break;
    case(ERR_LEN):
      BSP_Ser_Printf("Bad Packet Size");
      break;
    case(ERR_ADDRESS):
      BSP_Ser_Printf("Not My Address");
      break;
    case(ERR_MESSAGE_TYPE):
      BSP_Ser_Printf("Unknown Message Type");
      break;
    default:
      BSP_Ser_Printf("Unkown Exception");
      break;
  }
  
  BSP_Ser_Printf("\n\n");
}