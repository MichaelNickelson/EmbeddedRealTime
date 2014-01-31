#include "PktParser.h"
#include "BfrPair.h"

typedef struct
{
  CPU_INT08S payloadLen;
  CPU_INT08U data[1];
} PktBfr;

void ParsePkt(BfrPair *payloadBfrPair){
  
}