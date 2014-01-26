/*---------------------------- FILE: PktParser.c ----------------------------*/

#include "includes.h"

#include "PktParser.h"
#include "Errors.h"

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

void ParsePkt(void *pktBfr)
{
  CPU_INT08U preamble[HeaderLength-1] = {0x03, 0xEF, 0xAF};
  
  CPU_INT08U checkSum = 0;
  
  ParserState   parseState = P1;
  CPU_INT16S    c;
  CPU_INT08U    i;
  
  PktBfr *payloadBfr = pktBfr;
  for(;;){
    c = GetByte();
    
    checkSum ^= c;
//    BSP_Ser_Printf("%x", c);
//    BSP_Ser_Printf(" ");
    
    switch (parseState){
      case P1:
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
          parseState = L;
        }else{
          DispErr(ERR_PRE3);
          parseState = ER;
        }
        break;
      case L:
        payloadBfr->payloadLen = c - HeaderLength;
        if(c<ShortestPacket){
          DispErr(ERR_LEN);
          parseState = ER;
        }else{
        i = 0;
        parseState = R;
        }
        break;
      case R:
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