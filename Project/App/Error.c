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
#include "Constants.h"
#include "Framer.h"
#include "Memmgr.h"

#define ERR_PAYLOAD_SIZE 5

/*--------------- S e n d E r r o r ---------------
Generate an error message and send it to the framer for transmission
*/
void SendError(Error_t e){
  OS_ERR osErr;
  
  Buffer *eBfr = Allocate();
  
  BfrAddByte(eBfr, ERR_PAYLOAD_SIZE + PREAMBLE_LENGTH +1);
  BfrAddByte(eBfr, CtrlCtrAddress);
  BfrAddByte(eBfr, MyAddress);
  BfrAddByte(eBfr, MSG_ERR);
  BfrAddByte(eBfr, e);
  
  BfrClose(eBfr);
  OSQPost(&framerQueue, eBfr, sizeof(Buffer), OS_OPT_POST_FIFO, &osErr);
  assert(osErr == OS_ERR_NONE);
}
