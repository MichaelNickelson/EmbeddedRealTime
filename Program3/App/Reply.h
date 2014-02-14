#ifndef REPLY_H
#define REPLY_H
/*--------------- R e p l y . h ---------------

by: George Cheney
    ECE Dept.
    UMASS Lowell

PURPOSE
Copy messages from the Reply Buffer Pair Get Buffer to
the oBfrPair Put Buffer.

CHANGES
01-29-2013 gpc -  Created
*/

#include "BfrPair.h"

void PutReplyMsg(BfrPair *replyBfrPair, CPU_INT08U *msg);
void Reply(BfrPair *replyBfrPair);
#endif