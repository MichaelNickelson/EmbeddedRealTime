/*--------------- E r r o r . c ---------------

by: Michael Nickelson

PURPOSE
Generate error and assert messages

CHANGES
02/19/2014 mn - Initial submission
*/

#include "includes.h"
#include "Error.h"
#include "Payload.h"

/*--------------- D i s p E r r o r ---------------
Generate an error message sent to reply array and sent to reply buffer by
payload task.
*/
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

/*--------------- D i s p A s s e r t ---------------
Generate an assert message sent to reply array and sent to reply buffer by
payload task.
*/
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