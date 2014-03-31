/*--------------- S e r I O D r i v e r . c ---------------

by: Michael Nickelson

PURPOSE
This module sets up serial IO and appropriate buffers

CHANGES
02-19-2014 mn -  Initial submission
03-12-2014 mn -  Updated to use uCOS-III and semaphores
*/

#include "SerIODriver.h"
#include "assert.h"
#include "Buffer.h"

/*----- Constant definitions ----- */
#define RXNE_MASK 0x0020
#define TXE_MASK 0x0080
#define USART2ENA 0x00000040
#define TXEIE_MASK 0x0080
#define RXNEIE_MASK 0x0020
#define SETENA1 (*((CPU_INT32U *) 0xE000E104))
#define CLRENA1 (*((CPU_INT32U *) 0xE000E184))
#define NUM_BFRS 1
//#define SUSPEND_TIMEOUT (2 * BfrSize)
#define SUSPEND_TIMEOUT 25

/*----- Local Function prototypes -----*/
void ServiceRx();
void ServiceTx();
void ForceiBfr();

/*----- Global Variables -----*/
// Declare input and output buffer pairs
//static BfrPair iBfrPair;
BfrPair iBfrPair;
static CPU_INT08U iBfr0Space[BfrSize];
static CPU_INT08U iBfr1Space[BfrSize];

//static BfrPair oBfrPair;
BfrPair oBfrPair;
static CPU_INT08U oBfr0Space[BfrSize];
static CPU_INT08U oBfr1Space[BfrSize];

// Declare openObfrs and closedIBfrs semaphores
static OS_SEM openObfrs;
static OS_SEM closedIBfrs;

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
  
  // Initialize iBfrPair and oBfrPair
  BfrPairInit(&iBfrPair, iBfr0Space, iBfr1Space, BfrSize);
  BfrPairInit(&oBfrPair, oBfr0Space, oBfr1Space, BfrSize);
  
  // Initialize semaphores to be used by Serial communications driver
  OSSemCreate(&openObfrs, "Open oBfrs", NUM_BFRS, &osErr);
  assert(osErr == OS_ERR_NONE);
  OSSemCreate(&closedIBfrs, "Closed iBfrs", 0, &osErr);
  assert(osErr == OS_ERR_NONE);
}

/*----------- ServiceRx() -----------
If a new byte is available in the Status Register and the iBfrPair put 
buffer is open, grab it and put it into the iBfrPair PutBfr.
Swap buffers as needed.
*/
void ServiceRx(){
  USART_TypeDef *uart = USART2;
  OS_ERR osErr;
  
  if((uart->SR) & RXNE_MASK){
    if(!PutBfrClosed(&iBfrPair)){
      PutBfrAddByte(&iBfrPair, uart->DR);
      // If the put buffer closes, inform the OS.
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
Swap buffers as needed.
*/
void ServiceTx(){
  USART_TypeDef *uart = USART2;
  CPU_INT16S c;
  OS_ERR osErr;
  
  if((uart->SR) & TXE_MASK){
    if(GetBfrClosed(&oBfrPair)){
      c = GetBfrRemByte(&oBfrPair);
      uart->DR = c;
      
      // If the buffer opens, inform the OS
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
  
  if(!GetBfrClosed(&iBfrPair)){
    OSSemPend(&closedIBfrs, SUSPEND_TIMEOUT, OS_OPT_PEND_BLOCKING, NULL, &osErr);
    if(osErr == OS_ERR_TIMEOUT){
      ForceiBfr();
    }
    assert((osErr == OS_ERR_NONE) || (osErr == OS_ERR_TIMEOUT));
    
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
}

/*----------- BfrFlush() -----------
Flush the output buffer.
*/
void BfrFlush(void){
  OS_ERR osErr;
  
  OSSemPend(&openObfrs, SUSPEND_TIMEOUT, OS_OPT_PEND_BLOCKING, NULL, &osErr);
  assert(osErr == OS_ERR_NONE);
     
  ClosePutBfr(&oBfrPair);
  if(BfrPairSwappable(&oBfrPair))
    BfrPairSwap(&oBfrPair);
  
  while(GetBfrClosed(&oBfrPair))
    ServiceTx();
}

/*----------- ForceiBfr() -----------
Force the iBfr to send any available bytes
*/
void ForceiBfr(){
  OS_ERR osErr;
  
  if(iBfrPair.buffers[iBfrPair.putBrfNum].putIndex != 0){
    ClosePutBfr(&iBfrPair);
    OSSemPost(&closedIBfrs, OS_OPT_POST_1, &osErr);
  }
}