#ifndef PAYLOAD_H
#define PAYLOAD_H
/*--------------- P a y l o a d . h ---------------*/

#include "BfrPair.h"

CPU_INT16U Reverse2Bytes(CPU_INT16U b);
CPU_INT32U Reverse4Bytes(CPU_INT32U b);

void PayloadInit(BfrPair **payloadBfrPair, BfrPair **replyBfrPair);
void PayloadTask(void);

#endif