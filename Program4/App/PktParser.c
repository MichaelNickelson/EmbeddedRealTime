/*--------------- P k t P a r s e r . c ---------------

by: Michael Nickelson

PURPOSE
Handles incoming packets for payload buffer to parse.
Each state of the parsing state machine is implemented as a function that
receives a struct with the current state information.

CHANGES
02-19-2014 mn -  Initial Submission
03-12-2014 mn -  Updated to use uCOS-III and semaphores
*/

/* Include dependencies */
#include "includes.h"
#include "PktParser.h"
#include "assert.h"
#include "BfrPair.h"
#include "Error.h"
#include "Payload.h"
#include "SerIODriver.h"

/*----- c o n s t a n t    d e f i n i t i o n s -----*/
#define HeaderLength 4 
#define ShortestPacket 8
#define NUM_BFRS 2
#define SUSPEND_TIMEOUT 250
#define PARSER_STK_SIZE 128
#define ParserPrio 4
#define PayloadBfrSize 14
#define HIGH_WATER_LIMIT 10

/*----- t y p e d e f s   u s e d   i n   p a r s e r -----*/
/* Parser state data type */
typedef enum { P, L, R, ER } ParserState;

/* Relevant state information passed to state functions */
typedef struct{
  ParserState parseState;
  CPU_INT16S c;  // Current byte
  CPU_INT08U checkSum;
  CPU_INT08S payloadLen;
  CPU_INT08U preamble[HeaderLength-1];
} StateVariables_t;

/* Packet structure */
typedef struct{
  CPU_INT08U payloadLen;
  CPU_INT08U data[1];
} PktBfr;

/*----- G l o b a l   V a r i a b l e s -----*/
/*----- Initialize openPayloadBfrs and closedPayloadBfrs semaphores -----*/
OS_SEM openPayloadBfrs;
OS_SEM closedPayloadBfrs;

static OS_TCB parsePktTCB;
static CPU_STK parsePktStk[PARSER_STK_SIZE];

/*----- l o c a l   f u n c t i o n    p r o t o t y p e s -----*/
void DoStateP(StateVariables_t *myState);
void DoStateL(StateVariables_t *myState);
void DoStateR(StateVariables_t *myState);
void DoStateER(StateVariables_t *myState);
void ErrorTransition(StateVariables_t *myState);
void ParsePkt(void *payloadBfrPair);

/*--------------- C r e a t e P a r s e P k t T a s k ---------------
Start the packet parsing task and create the relevant semaphores
*/
void CreateParsePktTask(void){
  OS_ERR osErr;
  
  // Create semaphores and verify success
  OSSemCreate(&openPayloadBfrs, "Open payload buffers", NUM_BFRS, &osErr);
  assert(osErr == OS_ERR_NONE);
  OSSemCreate(&closedPayloadBfrs, "Closed payload buffers", 0, &osErr);
  assert(osErr == OS_ERR_NONE);
  
  // Start ParsePkt task and verify success
  OSTaskCreate(&parsePktTCB,
               "Packet parsing task",
               ParsePkt,
               NULL,
               ParserPrio,
               &parsePktStk[0],
               PARSER_STK_SIZE / HIGH_WATER_LIMIT,
               PARSER_STK_SIZE,
               0,
               0,
               (void *)0,
               0,
               &osErr);
  assert(osErr == OS_ERR_NONE);
}

/*--------------- P a r s e P k t ---------------
This is the main function used for packet parsing. It is implemented as a 
state machine.
*/
void ParsePkt(void *payloadBfrPair){

  static StateVariables_t myState = {.parseState = P,
                                     .c = 0,
                                     .checkSum = 0,
                                     .payloadLen = 0,
                                     .preamble = {0x03, 0xEF, 0xAF}};
  OS_ERR osErr;

  for(;;){
    // GetByte will pend if there is no data ready.
    myState.c = GetByte();
    
    // Maintain running checksum as bytes are received
    myState.checkSum ^= myState.c;
  
    switch (myState.parseState){
      case P:  // Look for a preamble
        DoStateP(&myState);
        break;
      case L: // Read in packet length
        // Since this is the first time the payload buffer is written to
        // and this state is only entered once, it makes sense to wait here
        // for a payload buffer to open.
        OSSemPend(&openPayloadBfrs, SUSPEND_TIMEOUT, OS_OPT_PEND_BLOCKING, NULL, &osErr);
        assert(osErr==OS_ERR_NONE);
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

/*--------------- D o S t a t e P ---------------
This state searches for a full preamble before moving on to state L.
If the wrong byte is found, it moves to the error state.
*/
void DoStateP(StateVariables_t *myState){
  static CPU_INT08S pb = 0;
  OS_ERR osErr;
  
  // If the wrong byte is found, go to error state
  if (myState->c != myState->preamble[pb++]){
    OSSemPend(&openPayloadBfrs, SUSPEND_TIMEOUT, OS_OPT_PEND_BLOCKING, NULL, &osErr);
    assert(osErr==OS_ERR_NONE);
    // Use preamble index that is currently being compared as the error code
    PutBfrAddByte(&payloadBfrPair, -(pb));
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
  if(myState->c<ShortestPacket){
    // Raise an error if the packet is too short
    PutBfrAddByte(&payloadBfrPair, ERR_LEN);
    ErrorTransition(myState);
  }else{
    // Calculate packet length
    myState->payloadLen = myState->c - HeaderLength;
    PutBfrAddByte(&payloadBfrPair, myState->payloadLen);
    myState->parseState = R;
  }
}

/*--------------- D o S t a t e R ---------------
Read in myState.payloadLen bytes, then validate the checksum and move on as 
appropriate.
*/
void DoStateR(StateVariables_t *myState){
  OS_ERR osErr;
  
  if(--myState->payloadLen > 0){
    PutBfrAddByte(&payloadBfrPair, myState->c);
  }else{
    if(myState->checkSum){
      // Reset put buffer so ERR_CHECKSUM is in the right place
      PutBfrReset(&payloadBfrPair);
      PutBfrAddByte(&payloadBfrPair, ERR_CHECKSUM);
      ErrorTransition(myState);
    }else{
      myState->parseState = P;
      ClosePutBfr(&payloadBfrPair);
      if(BfrPairSwappable(&payloadBfrPair))
        BfrPairSwap(&payloadBfrPair);
      // Inform the OS that a payload buffer was closed
      OSSemPost(&closedPayloadBfrs, OS_OPT_POST_1, &osErr);
      assert(osErr==OS_ERR_NONE);
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
  }else{ // If the wrong preamble byte is found, stay in error state
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
  OS_ERR osErr;
  
  ClosePutBfr(&payloadBfrPair);
  if(BfrPairSwappable(&payloadBfrPair))
    BfrPairSwap(&payloadBfrPair);
  // Inform the OS that a payload buffer was closed
  OSSemPost(&closedPayloadBfrs, OS_OPT_POST_1, &osErr);
  assert(osErr==OS_ERR_NONE);
  
  myState->checkSum=0;
  myState->parseState = ER;
}