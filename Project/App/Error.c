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
#include "Payload.h"

/*--------------- S e n d E r r o r ---------------
Generate an error message and send it to the framer for transmission
*/
void SendError(Error_t e, Buffer *eBfr){
  OS_ERR osErr;
  
  BfrReset(eBfr);
  
  BfrAddByte(eBfr, 9);
  BfrAddByte(eBfr, 1);
  BfrAddByte(eBfr, 2);
  BfrAddByte(eBfr, 0x0B);
  BfrAddByte(eBfr, e);
  
  BfrClose(eBfr);
  OSQPost(&framerQueue, eBfr, sizeof(Buffer), OS_OPT_POST_FIFO, &osErr);
  assert(osErr == OS_ERR_NONE);
}
