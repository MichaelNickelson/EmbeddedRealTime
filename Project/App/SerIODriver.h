/*--------------- S e r I O D r i v e r . h ---------------

by: Michael Nickelson

PURPOSE
Header for SerIODriver.c
This module sets up serial IO and appropriate buffers

CHANGES
02-19-2014 mn -  Initial submission
03-12-2014 mn -  Updated to use uCOS-III and semaphores
*/

#ifndef SERIODRIVER_H
#define SERIODRIVER_H

#include "includes.h"
#include "BfrPair.h"

/* Variable size for input and output buffers */
#ifndef BfrSize
#define BfrSize 4
#endif

//extern BfrPair oBfrPair;

/*----- f u n c t i o n    p r o t o t y p e s -----*/
void SerialISR(void);
void InitSerIO();
CPU_INT16S GetByte(void);
CPU_INT16S PutByte(CPU_INT16S txChar);
void BfrFlush(void);

#endif