/*---------------------------- FILE: PktParser.c ----------------------------*/

#include "includes.h"

#include "PktParser.h"

#define HeaderLength 4

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
          BSP_Ser_Printf("\a*** ERROR: Bad Preamble Byte 1\n\n");
          checkSum = 0;
          parseState = ER;
        }
        break;
      case P2:
        if (c == preamble[1]){
          parseState = P3;
        }else{
          BSP_Ser_Printf("\a*** ERROR: Bad Preamble Byte 2\n\n");
          checkSum = 0;
          parseState = ER;
        }
        break;
      case P3:
        if (c == preamble[2]){
          parseState = L;
        }else{
          BSP_Ser_Printf("\a*** ERROR: Bad Preamble Byte 3\n");
          checkSum = 0;
          parseState = ER;
        }
        break;
      case L:
        payloadBfr->payloadLen = c - HeaderLength;
        if(c<8){
          BSP_Ser_Printf("\a*** ERROR: Bad Packet Size\n\n");
          checkSum = 0;
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
            BSP_Ser_Printf("\a*** ERROR: Bad Checksum\n\n");
            checkSum = 0;
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
        }else{
          checkSum=0;
          parseState = ER;
        }
        break;
    }
  }
}