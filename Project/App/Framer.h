/*--------------- P k t P a r s e r . h ---------------

by: Michael Nickelson

PURPOSE - Header file
Handles incoming packets for payload buffer to parse.
Each state of the parsing state machine is implemented as a function that
receives a struct with the current state information.

CHANGES
02-19-2014 mn -  Initial submission
03-12-2014 mn -  ParsePkt is not needed by external modules, replaced with
                 CreateParsePktTask
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
//void MakePayload(CPU_INT08U receiver,
//                 CPU_INT08U type,
//                 CPU_INT08U payload);
void MakePayload(Buffer *payloadBfr,
                 CPU_INT08U receiver,
                 CPU_INT08U type,
                 CPU_INT08U payload);

#endif