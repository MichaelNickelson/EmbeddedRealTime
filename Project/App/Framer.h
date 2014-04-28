/*--------------- F r a m e r . h ---------------

by: Michael Nickelson

PURPOSE - header file
Handles sending outgoing packets to oBfrPair.

CHANGES
04-30-2014 - mn - Initial Submission
*/

#ifndef FRAMER_H
#define FRAMER_H

#include "Buffer.h"

extern OS_Q framerQueue; // The queue of parsed payloads to RobotManager

/*----- c o n s t a n t    d e f i n i t i o n s -----*/
// Also used by PktParser.c
#define PREAMBLE_LENGTH 3

/*----- f u n c t i o n    p r o t o t y p e s -----*/
void CreateFramerTask(void);
void SendAck(CPU_INT08U type);
void MakePayload(Buffer *payloadBfr,
                 CPU_INT08U receiver,
                 CPU_INT08U type,
                 CPU_INT08U payload);

#endif