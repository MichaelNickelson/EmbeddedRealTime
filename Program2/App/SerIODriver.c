/*--------------- S e r I O D r i v e r . c ---------------

by: Michael Nickelson

PURPOSE
This module sets up serial IO and appropriate buffers

CHANGES
02/19/2014 mn - Initial submission
*/

#include "SerIODriver.h"
#include "BfrPair.h"
#include "includes.h"
#include "Buffer.h"

/*----- Constant definitions ----- */
#define RXNE_MASK 0x0020
#define TXE_MASK 0x0080

#ifndef BfrSize
#define BfrSize 4
#endif

/*----- Function prototypes -----*/
void InitSerIO();
void ServiceRx();
void ServiceTx();
CPU_INT16S GetByte();
CPU_INT16S PutByte(CPU_INT16S txChar);

/*----- Declare input and output buffer pairs -----*/
BfrPair iBfrPair;
CPU_INT08U iBfr0Space[BfrSize];
CPU_INT08U iBfr1Space[BfrSize];

BfrPair oBfrPair;
CPU_INT08U oBfr0Space[BfrSize];
CPU_INT08U oBfr1Space[BfrSize];

/*----------- InitSerIO() -----------
Configure HW and initialize buffers
*/
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

/*----------- ServiceRx() -----------
If a new byte is available in the Status Register grab it and put it into the
iBfrPair PutBfr.
Swap buffers if needed.
*/
void ServiceRx(){
  USART_TypeDef *uart = USART2;
  
  if(((uart->SR) & RXNE_MASK) && (!PutBfrClosed(&iBfrPair)))
    PutBfrAddByte(&iBfrPair, uart->DR);
  
  if(BfrPairSwappable(&iBfrPair))
    BfrPairSwap(&iBfrPair);
  
  return;
}

/*----------- ServiceTx() -----------
If the Get buffer is closed, start dumping it out to the UART.
Swap buffers if needed.
*/
void ServiceTx(){
  USART_TypeDef *uart = USART2;
  
  if((GetBfrClosed(&oBfrPair)) && ((uart->SR) & TXE_MASK))
    uart->DR = GetBfrRemByte(&oBfrPair);
  
   if(BfrPairSwappable(&oBfrPair))
     BfrPairSwap(&oBfrPair);
}

/*----------- GetByte() -----------
Get a byte from iBfrPair if possible.
A response of -1 indicates an empty buffer.
*/
CPU_INT16S GetByte(){
  return GetBfrRemByte(&iBfrPair);
}

/*----------- PutByte() -----------
Send a byte to the output put buffer.
A negative return value indicates a full buffer
*/
CPU_INT16S PutByte(CPU_INT16S txChar){
  return PutBfrAddByte(&oBfrPair, txChar);;
}