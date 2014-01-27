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
typedef enum { P1, P2, P3, L, R, ER } ParserState;

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
  CPU_INT08U preamble[HeaderLength-1] = {0x03, 0xEF, 0xAF}; 
  CPU_INT08U checkSum = 0;
  ParserState   parseState = P1;
  CPU_INT16S    c;
  CPU_INT08U    i;
  PktBfr *payloadBfr = pktBfr;
  
  for(;;){
    c = GetByte();
    checkSum ^= c; // Maintain running checksum as packets are received
    
    switch (parseState){
      case P1:
        // Start looking for a packet, if anything is wrong, raise an error
        if (c == preamble[0]){
          parseState = P2;
        }else{
          DispErr(ERR_PRE1);
          parseState = ER;
        }
        break;
      case P2:
        if (c == preamble[1]){
          parseState = P3;
        }else{
          DispErr(ERR_PRE2);
          parseState = ER;
        }
        break;
      case P3:
        if (c == preamble[2]){
          // Found a full preamble, read in the packet length
          parseState = L;
        }else{
          DispErr(ERR_PRE3);
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
      case R:
        // Read in data
        payloadBfr->data[i++] = c;
        if (i>= payloadBfr->payloadLen){
          if(checkSum){
            DispErr(ERR_CHECKSUM);
            parseState = ER;
            break;
          }
          parseState = P1;
          return;
        }
        break;
      case ER:
        if (c == preamble[0]){
          parseState = P2;
          checkSum = c;
        }else{
          checkSum=0;
          parseState = ER;
        }
        break;
    }
  }
}