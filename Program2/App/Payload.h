#ifndef PAYLOAD_H
#define PAYLOAD_H
/*--------------- P a y l o a d . h ---------------*/

#include "BfrPair.h"

void PayloadInit(BfrPair **payloadBfrPair, BfrPair **replyBfrPair);
void PayloadTask(void);

#endif