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
#include "assert.h"

/*----- Constant definitions ----- */
#define RXNE_MASK 0x0020
#define TXE_MASK 0x0080
#define USART2ENA 0x00000040
#define TXEIE_MASK 0x0080
#define RXNEIE_MASK 0x0020
#define SETENA1 (*((CPU_INT32U *) 0xE000E104))
#define CLRENA1 (*((CPU_INT32U *) 0xE000E184))
#define NUM_BFRS 2

#define SUSPEND_TIMEOUT 100

/*----- Function prototypes -----*/
void SerialISR(void);
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

/*----- Initialize openObfrs and closedIBfrs semaphores -----*/
OS_SEM openObfrs;
OS_SEM closedIBfrs;

/*----------- SerialISR() -----------
Interrupt routine tripped by USART2
*/
void SerialISR(void){
  CPU_SR_ALLOC();
  OS_CRITICAL_ENTER();
  OSIntEnter();
  OS_CRITICAL_EXIT();
  
  ServiceRx();
  ServiceTx();
  
  OSIntExit();
}

/*----------- InitSerIO() -----------
Configure HW and initialize buffers
*/
void InitSerIO(){
  OS_ERR osErr;
  
  USART_TypeDef *uart = USART2;
  
  // Set UART baud rate to 9600bps
  uart->BRR = 0x0EA6;
  
  /* Enable UART, Tx, and Rx
  as well as interrupts. */
  uart->CR1 = 0x20AC;
  
  // Set 1 stop bit
  uart->CR2 = 0x0000;
  
  // Select full duplex mode
  uart->CR3 = 0x0000;
  
  static AFIO_TypeDef *afio = AFIO;
  
  // Remap USART
  afio->MAPR = 0x0008;
//  afio->MAPR = 0x0010;
  
  // Enable interrupts on USART2
  SETENA1 = USART2ENA;
  
  BfrPairInit(&iBfrPair, iBfr0Space, iBfr1Space, BfrSize);
  BfrPairInit(&oBfrPair, oBfr0Space, oBfr1Space, BfrSize);
  
  OSSemCreate(&openObfrs, "Open oBfrs", NUM_BFRS, &osErr);
  assert(osErr == OS_ERR_NONE);
  
  OSSemCreate(&closedIBfrs, "Closed iBfrs", 0, &osErr);
  assert(osErr == OS_ERR_NONE);
}

/*----------- ServiceRx() -----------
If a new byte is available in the Status Register grab it and put it into the
iBfrPair PutBfr.
Swap buffers if needed.
*/
void ServiceRx(){
  USART_TypeDef *uart = USART2;
  OS_ERR osErr;
  
  if((uart->SR) & RXNE_MASK){
    if(!PutBfrClosed(&iBfrPair)){
      PutBfrAddByte(&iBfrPair, uart->DR);
      if(PutBfrClosed(&iBfrPair)){
        OSSemPost(&closedIBfrs, OS_OPT_POST_1, &osErr);
        assert(osErr==OS_ERR_NONE);
      }
    }else{
      uart->CR1 = uart->CR1 ^ RXNEIE_MASK;
    }
  }
}

/*----------- ServiceTx() -----------
If the Get buffer is closed, start dumping it out to the UART.
Swap buffers if needed.
*/
void ServiceTx(){
  USART_TypeDef *uart = USART2;
  CPU_INT16S c;
  OS_ERR osErr;
  
  if((uart->SR) & TXE_MASK){
    if(GetBfrClosed(&oBfrPair)){
      c = GetBfrRemByte(&oBfrPair);
      uart->DR = c;
      
      if(!GetBfrClosed(&oBfrPair)){
        OSSemPost(&openObfrs, OS_OPT_POST_1, &osErr);
        assert(osErr==OS_ERR_NONE);
      }
    }else{
      uart->CR1 = uart->CR1 ^ TXEIE_MASK;
    }
  }
}

/*----------- GetByte() -----------
Get a byte from iBfrPair if possible.
A response of -1 indicates an empty buffer.
*/
CPU_INT16S GetByte(){
  CPU_INT16S retVal = -1;
  USART_TypeDef *uart = USART2;
  OS_ERR osErr;
  
//  if(BfrPairSwappable(&iBfrPair))
//    BfrPairSwap(&iBfrPair);
//  
//  if(GetBfrClosed(&iBfrPair)){
//    uart->CR1 = uart->CR1 | RXNEIE_MASK;
//    retVal = GetBfrRemByte(&iBfrPair);
//  }
  
  if(!GetBfrClosed(&iBfrPair)){
//    OSSemPend(&closedIBfrs, SUSPEND_TIMEOUT, OS_OPT_PEND_BLOCKING, NULL, &osErr);
    OSSemPend(&closedIBfrs, 0, OS_OPT_PEND_BLOCKING, NULL, &osErr);
    assert(osErr == OS_ERR_NONE);
    
    if(BfrPairSwappable(&iBfrPair))
      BfrPairSwap(&iBfrPair);
  }
     
  if(GetBfrClosed(&iBfrPair)){
    uart->CR1 = uart->CR1 | RXNEIE_MASK;
    retVal = GetBfrRemByte(&iBfrPair);
  }
  
  return retVal;
}

/*----------- PutByte() -----------
Send a byte to the output put buffer.
A negative return value indicates a full buffer
*/
CPU_INT16S PutByte(CPU_INT16S txChar){
  CPU_INT16S retVal = -1;
  USART_TypeDef *uart = USART2;
  OS_ERR osErr;

  if(PutBfrClosed(&oBfrPair)){
    OSSemPend(&openObfrs, SUSPEND_TIMEOUT, OS_OPT_PEND_BLOCKING, NULL, &osErr);
    assert(osErr == OS_ERR_NONE);
    
    if(BfrPairSwappable(&oBfrPair))
      BfrPairSwap(&oBfrPair);
  }
  
  PutBfrAddByte(&oBfrPair,txChar);
  retVal = txChar;
  uart->CR1 = uart->CR1 | TXEIE_MASK;
  
  return retVal;
  
//  return PutBfrAddByte(&oBfrPair, txChar);
}