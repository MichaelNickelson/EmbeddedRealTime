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
  static CPU_INT08U checkSum = 0, payloadLen;
  static CPU_INT08S pb = 0;
  CPU_INT08U preamble[HeaderLength-1] = {0x03, 0xEF, 0xAF};
  CPU_INT16S c;

  c = GetByte();
  
    if(c!=-1){
    checkSum ^= c; // Maintain running checksum as bytes are received
    
    switch (parseState){
      case P:  // Look for a preamble
        if (c == preamble[pb]){
          pb++;
        }else{ // If the wrong byte is found, go to error state
          PutBfrAddByte(payloadBfrPair, -(pb+1));
          ClosePutBfr(payloadBfrPair);
          if(BfrPairSwappable(payloadBfrPair))
            BfrPairSwap(payloadBfrPair);
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
          PutBfrAddByte(payloadBfrPair, ERR_LEN);
          ClosePutBfr(payloadBfrPair);
          if(BfrPairSwappable(payloadBfrPair))
            BfrPairSwap(payloadBfrPair);
          checkSum=0;
          parseState = ER;
        }else{
          payloadLen = c - HeaderLength; // Calculate packet length
          PutBfrAddByte(payloadBfrPair, payloadLen);
          parseState = R;
        }
        break;
      case R:   // Read in data
        if(payloadBfrPair->buffers[payloadBfrPair->putBrfNum].putIndex < payloadLen){
          PutBfrAddByte(payloadBfrPair, c);
        }else{
          if(checkSum){
            checkSum = 0;
            PutBfrReset(payloadBfrPair);
            PutBfrAddByte(payloadBfrPair, ERR_CHECKSUM);
            ClosePutBfr(payloadBfrPair);
            if(BfrPairSwappable(payloadBfrPair))
               BfrPairSwap(payloadBfrPair);
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
        if(pb >= HeaderLength-1){
          parseState = L; // Move on if preamble found
          pb = 0;
        }
        break;
    }
  }
}