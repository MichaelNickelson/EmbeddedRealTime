/*--------------- R e p l y . c ---------------

by: George Cheney
    ECE Dept.
    UMASS Lowell

PURPOSE
This module defines the reply task.

CHANGES
01-29-2013 gpc -  Created
*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "includes.h"

#include "Reply.h"
#include "BfrPair.h"
#include "SerIODriver.h"

/*----- c o n s t a n t    d e f i n i t i o n s -----*/

/*--------------- P u t R e p l y M s g ( ) ---------------

PURPOSE
Append a message to the reply buffer pair put buffer.

INPUT PARAMETERS
replyBfrPair  - the address of the Reply Buffer Pair
msg           - the address of the message string
*/

void  PutReplyMsg(BfrPair *replyBfrPair, CPU_INT08U *msg)
{
  while (*msg != '\0' && !PutBfrClosed(replyBfrPair))
    PutBfrAddByte(replyBfrPair, *msg++);
}

/*--------------- R e p l y ( ) ---------------

PURPOSE
This is the reply task, which outputs the reply buffer pair to the
RS232 transmit port.

*/

void Reply(BfrPair *replyBfrPair)
{
  // If reply buffers are ready to swap, swap them.
  if (BfrPairSwappable(replyBfrPair))
    BfrPairSwap(replyBfrPair);
  
  // If the Get Buffer is not ready, can't proceed.
  if (!GetBfrClosed(replyBfrPair))
    return;
  
  // Copy bytes from the Reply Buffer Pair Get Buffer to 
  // the oBfrPair Put Buffer until  the Reply Get Buffer is
  // empty.
  while (GetBfrClosed(replyBfrPair))
    {
    // Get the next byte from the Get Buffer.
    CPU_INT16S c = GetBfrNextByte(replyBfrPair);
    
    // If the Get Buffer is empty, return.
    if (c < 0)
      return;
    
    // Try to copy the byte to the Put Buffer. If the Put Buffer is full, return.
    if (PutByte(c) < 0)
      return;
    
    // Remove the copied byte from the Get Buffer.
    GetBfrRemByte(replyBfrPair);
    }
}  
  
