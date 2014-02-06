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
CPU_INT08U iBfr0Space[BfrSize];
CPU_INT08U iBfr1Space[BfrSize];

static BfrPair oBfrPair;
static CPU_INT08U oBfr0Space[BfrSize];
static CPU_INT08U oBfr1Space[BfrSize];

void InitSerIO(){
  USART_TypeDef *uart = USART2;
  
  // Set UART baud rate to 9600bps
  uart->BRR = 0x0EA6;
  
  // Enable UART, Tx, and Rx
  uart->CR1 = 0x200C;
  
  // Set 1 stop bit
  uart->CR2 = 0x0000;
  
  // Select full duplex mode
  uart->CR3 = 0x0000;
  
  static AFIO_TypeDef *afio = AFIO;
  
  // Remap USART
  afio->MAPR = 0x0008;
//  afio->MAPR = 0x0010;
  
  BfrPairInit(&iBfrPair, iBfr0Space, iBfr1Space, BfrSize);
  BfrPairInit(&oBfrPair, oBfr0Space, oBfr1Space, BfrSize);
  
  return;
}

void ServiceRx(){
  USART_TypeDef *uart = USART2;
  
  if(((uart->SR) & RXNE_MASK) && (!PutBfrClosed(&iBfrPair)))
    PutBfrAddByte(&iBfrPair, uart->DR);
  
  if(BfrPairSwappable(&iBfrPair))
    BfrPairSwap(&iBfrPair);
  
  return;
}

void ServiceTx(){
  USART_TypeDef *uart = USART2;
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
  return PutBfrAddByte(&oBfrPair, txChar);;
}