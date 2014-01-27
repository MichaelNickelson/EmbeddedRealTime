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
typedef enum { P0 = 0, P1 = 1, P2 = 2, L = 3, R = 4, ER = 5 } ParserState;

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
  ParserState   parseState = P0;
  CPU_INT16S    c;
  CPU_INT08U    i, checkSum = 0, preamble[HeaderLength-1] = {0x03, 0xEF, 0xAF};
  PktBfr *payloadBfr = pktBfr;
  
  for(;;){
    c = GetByte();
    checkSum ^= c; // Maintain running checksum as packets are received
    
    switch (parseState){
      case P0:  // Start looking for a preamble, 
      case P1:  // if anything is wrong,
      case P2:  // raise an error.
        if (c == preamble[parseState]){
          parseState++;
        }else{
          PreambleError(parseState+1);
          parseState = ER;
        }
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
          parseState = P0;
          return;
        }
        break;
      case ER:  // If an error occurs, or a an unknown state arises,
      default:  // continue looking for preamble bytes.
        if (c == preamble[0]){
          parseState = P1;
          checkSum = c;
        }else{
          parseState = ER;
        }
        break;
    }
  }
}