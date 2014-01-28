/*---------------------------- FILE: PktParser.c ----------------------------*/

/* Include Micrium and STM headers. */
#include "includes.h"

/* Include own header and header for error handling */
#include "PktParser.h"
#include "Errors.h"

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
void ParsePkt(void *pktBfr)
{
  /* Initialize variables needed for packet parsing */
  ParserState   parseState = P;
  CPU_INT16S    c;
  CPU_INT08U    i, p = 0, checkSum = 0, preamble[HeaderLength-1] = {0x03, 0xEF, 0xAF};
  PktBfr *payloadBfr = pktBfr;
  
  for(;;){
    c = GetByte();
    checkSum ^= c; // Maintain running checksum as packets are received
    
    switch (parseState){
      case P:  // Start looking for a preamble
        if (c == preamble[p]){
          p++;
        }else{ // If the wrong byte is found, go to error state
          PreambleError(p+1);
          parseState = ER;
        }
        if (p >= HeaderLength-1) // Once the full header is found, move on
          parseState = L;
        break;
      case L:
        // Read in packet length, subtract header length to get payload length
        payloadBfr->payloadLen = c - HeaderLength;
        if(c<ShortestPacket){ // Raise an error if the packet is too short
          DispErr(ERR_LEN);
          parseState = ER;
        }else{
        i = 0; // Start reading in data, starting with byte 0
        parseState = R;
        }
        break;
      case R:   // Read in data
        payloadBfr->data[i++] = c;
        if (i>= payloadBfr->payloadLen){
          if(checkSum){
            DispErr(ERR_CHECKSUM);
            parseState = ER;
            break;
          }
          parseState = P;
          p=0;
          return;
        }
        break;
      case ER:  // If an error occurs, or a an unknown state arises,
      default:  // continue looking for a preamble.
        if (c == preamble[0]){
          parseState = P;
          p=1;
          checkSum = c;
        }else{
          parseState = ER;
        }
        break;
    }
  }
}