#include "SerIODriver.h"
#include "BfrPair.h"
#include "includes.h"
#include "Buffer.h"

#define RXNE_MASK 0x0020
#define TXE_MASK 0x0080

#ifndef BfrSize
#define BfrSize 4
#endif

BfrPair iBfrPair;
CPU_INT08U iBfr0Space[4];
CPU_INT08U iBfr1Space[4];

static BfrPair oBfrPair;
static CPU_INT08U oBfr0Space[4];
static CPU_INT08U oBfr1Space[4];

void InitSerIO(){
  static USART_TypeDef *uart = USART2;
  
  uart->BRR = 0x0EA6;
  uart->CR1 = 0x200C;
  uart->CR2 = 0x0000;
  uart->CR3 = 0x0000;
  
  static AFIO_TypeDef *afio = AFIO;
  afio->MAPR = 0x0008;
//  afio->MAPR = 0x0010;
  
  BfrPairInit(&iBfrPair, iBfr0Space, iBfr1Space, 4);
  BfrPairInit(&oBfrPair, oBfr0Space, oBfr1Space, 4);
  
  return;
}

void ServiceRx(){
  static USART_TypeDef *uart = USART2;
  
  if(((uart->SR) & RXNE_MASK) && (!PutBfrClosed(&iBfrPair)))
    PutBfrAddByte(&iBfrPair, uart->DR);
  
  if(BfrPairSwappable(&iBfrPair))
    BfrPairSwap(&iBfrPair);
  
  return;
}

void ServiceTx(){
  static USART_TypeDef *uart = USART2;
  CPU_INT16U byte;
  if(BfrPairSwappable(&oBfrPair))
     BfrPairSwap(&oBfrPair);
  if(((uart->SR) & TXE_MASK) && (GetBfrClosed(&oBfrPair))){
    byte = GetBfrRemByte(&oBfrPair);
    uart->DR = byte;
  }
}

CPU_INT16S GetByte(void){
  return GetBfrRemByte(&iBfrPair);

}

CPU_INT16S PutByte(CPU_INT16S txChar){
  CPU_INT16S retVal = PutBfrAddByte(&oBfrPair, txChar);
  return retVal;
}