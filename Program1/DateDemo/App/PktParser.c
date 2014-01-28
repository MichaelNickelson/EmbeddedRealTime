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
  CPU_INT08U    i, pb = 0, checkSum = 0, preamble[HeaderLength-1] = {0x03, 0xEF, 0xAF};
  PktBfr *payloadBfr = pktBfr;
  
  for(;;){
    c = GetByte();
    checkSum ^= c; // Maintain running checksum as packets are received
    
//    BSP_Ser_Printf(" %x",c);
    
    switch (parseState){
      case P:  // Start looking for a preamble
        if (c == preamble[pb]){
          pb++;
        }else{ // If the wrong byte is found, go to error state
          PreambleError(pb+1);
          pb = 0;
          checkSum=0;
          parseState = ER;
        }
        if (pb >= HeaderLength-1){ // Once the full header is found, move on
          parseState = L;
          pb = 0;
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
//        if (i> payloadBfr->payloadLen){
          if(checkSum){
            DispErr(ERR_CHECKSUM);
            checkSum = 0;
//            parseState = ER;
            parseState = P;
            break;
          }
          parseState = P;
          pb=0;
          return;
        }
        break;
      case ER:  // If an error occurs, or a an unknown state arises,
      default:  // continue looking for a preamble.
//        if (c == preamble[0]){
//          parseState = P;
//          pb=1;
//          checkSum = c;
//        }else{
//          parseState = ER;
//        }
//        break;
        if (c == preamble[pb]){
          pb++;
        }else{ // If the wrong byte is found, go to error state
          pb = 0;
          checkSum = 0;
        }
        if (pb >= HeaderLength-1) // Once the full header is found, move on
          parseState = L;
        break;
    }
  }
}