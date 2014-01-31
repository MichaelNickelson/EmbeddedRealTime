#ifndef SERIODRIVER_H
#define SERIODRIVER_H
/*--------------- S e r I O D r i v e r . h --------------- */

#include "includes.h"

void InitSerIO();
void ServiceRx();
void ServiceTx();

CPU_INT16S GetByte(void);
CPU_INT16S PutByte(CPU_INT16S txChar);

#endif