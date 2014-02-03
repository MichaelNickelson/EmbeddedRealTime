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

/* Packet structure */
typedef struct
{
  CPU_INT08U payloadLen;
  CPU_INT08U data[1];
} PktBfr;

/* Function for packet handling */
void ParsePkt(BfrPair *payloadBfrPair){
  static ParserState parseState = P;
  static CPU_INT08U pb = 0, checkSum = 0, i = 0, payloadLen;
//  BfrPair iBfrPair;
  // Initialize variables
  CPU_INT16S    c;
  CPU_INT08U    preamble[HeaderLength-1] = {0x03, 0xEF, 0xAF};

  if(GetBfrClosed(&iBfrPair)){
    c = GetBfrRemByte(&iBfrPair);
    PutBfrAddByte(payloadBfrPair, c);
//  }
  if(!PutBfrClosed(payloadBfrPair))
//    checkSum ^= c; // Maintain running checksum as bytes are received
    
    switch (parseState){
      case P:  // Look for a preamble
        if (c == preamble[pb]){
          pb++;
        }else{ // If the wrong byte is found, go to error state
          PreambleError(pb+1);
          checkSum=0;
          pb = 0;
          parseState = ER;
        }
        if (pb >= HeaderLength-1){ // Once the full header is found, move on
          pb = 0;
          parseState = L;
        }
        break;
      case L: // Read in packet length
        if(c<ShortestPacket){ // Raise an error if the packet is too short
          DispErr(ERR_LEN);
          parseState = ER;
        }else{
//          payloadLen = c - HeaderLength; // Calculate packet length
          payloadLen = c; // Calculate packet length
//          i = 0; // Start reading in data, starting with byte 0
          parseState = R;
        }
        break;
      case R:   // Read in data
        if(payloadBfrPair->buffers[payloadBfrPair->putBrfNum].putIndex >= payloadLen){
          if(checkSum){
            DispErr(ERR_CHECKSUM);
            checkSum = 0;
            parseState = ER;
            break;
          }
          parseState = P;
          ClosePutBfr(payloadBfrPair);
          if(BfrPairSwappable(payloadBfrPair))
            BfrPairSwap(payloadBfrPair);
        }
        break;
      case ER:  // If an error occurs, or a an unknown state arises,
      default:  // look for a  full preamble.
        if (c == preamble[pb]){
          pb++;
        }else{ // If the wrong byte is found, stay in error state
          pb = 0;
          checkSum = 0;
        }
        parseState = (pb >= HeaderLength-1) ?L:ER; // Move on if preamble found
        break;
    }
  }
}