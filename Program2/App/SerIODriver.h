#ifndef SERIODRIVER_H
#define SERIODRIVER_H
/*--------------- S e r I O D r i v e r . h --------------- */

#include "includes.h"
#include "BfrPair.h"

//static BfrPair iBfrPair;
//static CPU_INT08U iBfr0Space[4];
//static CPU_INT08U iBfr1Space[4];

extern BfrPair iBfrPair;
extern CPU_INT08U iBfr0Space[4];
extern CPU_INT08U iBfr1Space[4];

extern BfrPair oBfrPair;
extern CPU_INT08U oBfr0Space[4];
extern CPU_INT08U oBfr1Space[4];

void InitSerIO();
void ServiceRx();
void ServiceTx();

CPU_INT16S GetByte(void);
CPU_INT16S PutByte(CPU_INT16S txChar);

#endif