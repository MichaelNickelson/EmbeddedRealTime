/*--------------- P k t P a r s e r . c ---------------

by: Michael Nickelson

PURPOSE
Handles incoming packets for payload buffer to parse.
Each state of the parsing state machine is implemented as a function that
receives a struct with the current state information.

CHANGES
02-19-2014 mn -  Initial Submission
*/

/* Include dependencies */
#include "includes.h"
#include "SerIODriver.h"
#include "PktParser.h"
#include "Error.h"
#include "BfrPair.h"

/*----- c o n s t a n t    d e f i n i t i o n s -----*/
#define HeaderLength 4 
#define ShortestPacket 8

/*----- t y p e d e f s   u s e d   i n   p a r s e r -----*/
/* Parser state data type */
typedef enum { P, L, R, ER } ParserState;

/* Relevant state information passed to state functions */
typedef struct{
  ParserState parseState;
  CPU_INT16S c;  // Current byte
  CPU_INT08U checkSum;
  CPU_INT08S payloadLen;
  BfrPair* payloadBfrPair;
  CPU_INT08U preamble[HeaderLength-1];
} StateVariables_t;

/* Packet structure */
typedef struct{
  CPU_INT08U payloadLen;
  CPU_INT08U data[1];
} PktBfr;

/*----- f u n c t i o n    p r o t o t y p e s -----*/
void DoStateP(StateVariables_t *myState);
void DoStateL(StateVariables_t *myState);
void DoStateR(StateVariables_t *myState);
void DoStateER(StateVariables_t *myState);
void ErrorTransition(StateVariables_t *myState);

/*--------------- P a r s e P k t ---------------
This is the main function used for packet parsing. It is implemented as a 
state machine.
*/
void ParsePkt(BfrPair *payloadBfrPair){

  static StateVariables_t myState = {.parseState = P,
                                     .c = 0,
                                     .checkSum = 0,
                                     .payloadLen = 0,
                                     .preamble = {0x03, 0xEF, 0xAF}};
  myState.payloadBfrPair = payloadBfrPair;

  // Only begin parsing if source ready and destination ready conditions are met
  if(GetBfrClosed(&iBfrPair)&!PutBfrClosed(payloadBfrPair)){
    myState.c = GetByte();
    
    // If a byte is available run through the state machine.
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

/*--------------- D o S t a t e P ---------------
This state searches for a full preamble before moving on to state L.
If the wrong byte is found, it moves to the error state.
*/
void DoStateP(StateVariables_t *myState){
  static CPU_INT08S pb = 0;
  
  // If the wrong byte is found, go to error state
  if (myState->c != myState->preamble[pb++]){
    // Use preamble index that is currently being compared as the error code
    PutBfrAddByte(myState->payloadBfrPair, -(pb));
    ErrorTransition(myState);
    pb = 0;
  }
  
  // Once the full header is found, move to the next state
  if (pb >= HeaderLength-1){
    pb = 0;
    myState->parseState = L;
  }
}

/*--------------- D o S t a t e L ---------------
Read in the length of the packet. If it's too short, raise an error.
*/
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

/*--------------- D o S t a t e R ---------------
Read in myState.payloadLen bytes, then validate the checksum and move on as 
appropriate.
*/
void DoStateR(StateVariables_t *myState){
  if(--myState->payloadLen > 0){
     PutBfrAddByte(myState->payloadBfrPair, myState->c);
  }else{
    if(myState->checkSum){
      // Reset put buffer so ERR_CHECKSUM is in the right place
      PutBfrReset(myState->payloadBfrPair);
      PutBfrAddByte(myState->payloadBfrPair, ERR_CHECKSUM);
      ErrorTransition(myState);
    }else{
      myState->parseState = P;
      ClosePutBfr(myState->payloadBfrPair);
      if(BfrPairSwappable(myState->payloadBfrPair))
        BfrPairSwap(myState->payloadBfrPair);
    }
  }
}

/*--------------- D o S t a t e E R ---------------
Do not leave or report another error until a full preamble is found.
*/
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

/*--------------- E r r o r T r a n s i t i o n ---------------
Called when an error is found to handle progression to error state.
Close and swap buffers, reset state variables as needed
*/
void ErrorTransition(StateVariables_t *myState){
    ClosePutBfr(myState->payloadBfrPair);
    if(BfrPairSwappable(myState->payloadBfrPair))
      BfrPairSwap(myState->payloadBfrPair);
    myState->checkSum=0;
    myState->parseState = ER;
}