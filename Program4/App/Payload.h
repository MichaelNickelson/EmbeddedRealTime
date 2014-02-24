/*--------------- P a y l o a d . c ---------------

by: Michael Nickelson

PURPOSE - Header file
Parse payload and handle response

CHANGES
02-19-2014 mn -  Initial submission
03-12-2014 mn -  Updated to use uCOS-III and semaphores
*/

#ifndef PAYLOAD_H
#define PAYLOAD_H

#include "BfrPair.h" // Needed for payloadBfrPair

// Allow payloadBfrPair to be used by PktParser
extern BfrPair payloadBfrPair;

/*----- f u n c t i o n    p r o t o t y p e s -----*/
void CreatePayloadTask(void);

#endif