/*--------------- S e r I O D r i v e r . h ---------------

by: Michael Nickelson

PURPOSE
Header for SerIODriver.c
This module sets up serial IO and appropriate buffers

CHANGES
02/19/2014 mn - Initial submission
*/

#ifndef SERIODRIVER_H
#define SERIODRIVER_H

#include "includes.h"
#include "BfrPair.h"

/* Variable size for input and output buffers */
#ifndef BfrSize
#define BfrSize 4
#endif

/*----- G l o b a l   D e c l a r a t i o n s -----*/
/* iBfrPair and oBfrPair are needed by other modules */
extern BfrPair iBfrPair;
extern CPU_INT08U iBfr0Space[BfrSize];
extern CPU_INT08U iBfr1Space[BfrSize];

extern BfrPair oBfrPair;
extern CPU_INT08U oBfr0Space[BfrSize];
extern CPU_INT08U oBfr1Space[BfrSize];


/*----- f u n c t i o n    p r o t o t y p e s -----*/
void SerialISR(void);
void InitSerIO();
void ServiceRx();
void ServiceTx();
CPU_INT16S GetByte(void);
CPU_INT16S PutByte(CPU_INT16S txChar);

#endif