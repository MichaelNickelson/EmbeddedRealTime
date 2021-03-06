/*---------------------------- FILE: PktParser.c ----------------------------*/

/* Include Micrium and STM headers. */
#include "includes.h"

/* Include own header and header for error handling */
#include "PktParser.h"
#include "Errors.h"

/* Define parameters of packet */
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
void ParsePkt(void *pktBfr)
{
  ParserState   parseState = P; // Initialize variables
  CPU_INT16S    c;
  CPU_INT08U    i, pb = 0, checkSum = 0, preamble[HeaderLength-1] = {0x03, 0xEF, 0xAF};
  PktBfr *payloadBfr = pktBfr;
  
  for(;;){
    c = GetByte();
    checkSum ^= c; // Maintain running checksum as bytes are received
    
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
          payloadBfr->payloadLen = c - HeaderLength; // Calculate packet length
          i = 0; // Start reading in data, starting with byte 0
          parseState = R;
        }
        break;
      case R:   // Read in data
        payloadBfr->data[i++] = c;
        if (i>= payloadBfr->payloadLen){
          if(checkSum){
            DispErr(ERR_CHECKSUM);
            checkSum = 0;
            parseState = ER;
            break;
          }
          parseState = P;
          return;
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