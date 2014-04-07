/*--------------- E r r o r . c ---------------

by: Michael Nickelson

PURPOSE
Generate error and assert messages

CHANGES
02/19/2014 mn - Initial submission
*/

#include "includes.h"
#include "Error.h"
#include "assert.h"
#include "Framer.h"
#include "Memmgr.h"
#include "RobotControl.h"

////#define ERR_PAYLOAD_SIZE 5

/*--------------- S e n d E r r o r ---------------
Generate an error message and send it to the framer for transmission
*/
void SendError(Error_t e){
  
  Buffer *eBfr = Allocate();
  
  MakePayload(eBfr, CtrlCtrAddress, MSG_ERR, e);
}
