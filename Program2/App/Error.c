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
      sprintf(reply, "\a*** ERROR: Unkown Message Type\n");
      break;
  }
}

void DispAssert(Assert_t a, CPU_CHAR reply[]){

  switch(a){
    case(ASS_ADDRESS):
      sprintf(reply, "\a*** Info: Not My Address\n");
      break;
    default:
      sprintf(reply, "\a*** Unknown Assertion\n");
      break;
  }
}