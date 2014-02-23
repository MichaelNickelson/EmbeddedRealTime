/*--------------- P k t P a r s e r . h ---------------

by: Michael Nickelson

PURPOSE
This module sets up serial IO and appropriate buffers

CHANGES
02/19/2014 mn - Initial submission
*/

#ifndef PKTPARSER_H
#define PKTPARSER_H

#include "BfrPair.h"

extern OS_SEM openPayloadBfrs;
extern OS_SEM closedPayloadBfrs;

/*----- f u n c t i o n    p r o t o t y p e s -----*/
void ParsePkt(void *payloadBfrPair);
void CreateParsePktTask(void);

#endif