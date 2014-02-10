/*---------------------------- FILE: PktParser.c ----------------------------*/

/* Include Micrium and STM headers. */
#include "includes.h"

/* Include own header and header for error handling */
#include "SerIODriver.h"
#include "PktParser.h"
#include "Error.h"
#include "BfrPair.h"

/* Define parameters of packet */
#define HeaderLength 4 
#define ShortestPacket 8

/* Parser state data type */
typedef enum { P, L, R, ER } ParserState;

typedef struct
{
  ParserState parseState;
  CPU_INT16S c;
  CPU_INT08U checkSum;
  CPU_INT08S payloadLen;
  BfrPair* payloadBfrPair;
  CPU_INT08U preamble[HeaderLength-1];
} StateVariables_t;

/* Packet structure */
typedef struct
{
  CPU_INT08U payloadLen;
  CPU_INT08U data[1];
} PktBfr;

void DoStateP(StateVariables_t *myState);
void DoStateL(StateVariables_t *myState);
void DoStateR(StateVariables_t *myState);
void DoStateER(StateVariables_t *myState);
void ErrorTransition(StateVariables_t *myState);

/* Function for packet handling */
void ParsePkt(BfrPair *payloadBfrPair){
  
  static StateVariables_t myState = {.parseState = P,
                                     .c = 0,
                                     .checkSum = 0,
                                     .payloadLen = 0,
                                     .preamble = {0x03, 0xEF, 0xAF}};
  myState.payloadBfrPair = payloadBfrPair;

  if(GetBfrClosed(&iBfrPair)&!PutBfrClosed(payloadBfrPair)){
    myState.c = GetByte();
    
    if(myState.c!=-1){
    myState.checkSum ^= myState.c; // Maintain running checksum as bytes are received
    
      switch (myState.parseState){
        case P:  // Look for a preamble
          DoStateP(&myState);
          break;
        case L: // Read in packet length
          DoStateL(&myState);
          break;
        case R:   // Read in data
          DoStateR(&myState);
          break;
        case ER:  // If an error occurs, or a an unknown state arises,
        default:  // look for a  full preamble.
          DoStateER(&myState);
          break;
      }
    }
  }
}

void DoStateP(StateVariables_t *myState){
  static CPU_INT08S pb = 0;
  
  if (myState->c != myState->preamble[pb++]){ // If the wrong byte is found, go to error state
    PutBfrAddByte(myState->payloadBfrPair, -(pb));
    ErrorTransition(myState);
    pb = 0;
  }
  if (pb >= HeaderLength-1){ // Once the full header is found, move on
    pb = 0;
    myState->parseState = L;
  }
}

void DoStateL(StateVariables_t *myState){
  if(myState->c<ShortestPacket){ // Raise an error if the packet is too short
    PutBfrAddByte(myState->payloadBfrPair, ERR_LEN);
    ErrorTransition(myState);
  }else{
    myState->payloadLen = myState->c - HeaderLength; // Calculate packet length
    PutBfrAddByte(myState->payloadBfrPair, myState->payloadLen);
    myState->parseState = R;
  }
}

void DoStateR(StateVariables_t *myState){
  if(--myState->payloadLen > 0){
     PutBfrAddByte(myState->payloadBfrPair, myState->c);
  }else{
    if(myState->checkSum){
      PutBfrReset(myState->payloadBfrPair);
      PutBfrAddByte(myState->payloadBfrPair, ERR_CHECKSUM);
      ErrorTransition(myState);
      return;
    }
    myState->parseState = P;
    ClosePutBfr(myState->payloadBfrPair);
    if(BfrPairSwappable(myState->payloadBfrPair))
      BfrPairSwap(myState->payloadBfrPair);
  }
}

void DoStateER(StateVariables_t *myState){
  static CPU_INT08S pb = 0;
  if (myState->c == myState->preamble[pb]){
    pb++;
  }else{ // If the wrong byte is found, stay in error state
    pb = 0;
    myState->checkSum = 0;
  }
  if(pb >= HeaderLength-1){
    myState->parseState = L; // Move on if preamble found
    pb = 0;
  }
}

void ErrorTransition(StateVariables_t *myState){
    ClosePutBfr(myState->payloadBfrPair);
    if(BfrPairSwappable(myState->payloadBfrPair))
      BfrPairSwap(myState->payloadBfrPair);
    myState->checkSum=0;
    myState->parseState = ER;
}