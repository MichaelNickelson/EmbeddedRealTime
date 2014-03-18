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

#ifndef PKTPARSER_H
#define PKTPARSER_H

//// Allow semaphores to be used by Payload.c
//extern OS_SEM openPayloadBfrs;
//extern OS_SEM closedPayloadBfrs;
extern OS_Q parserQueue;

/*----- f u n c t i o n    p r o t o t y p e s -----*/
void CreateParsePktTask(void);

#endif