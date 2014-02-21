/*--------------- P r o g 4 . c ---------------

By: Michael Nickelson

PURPOSE
Receive wireless sensor network packets from the RS232 port,
interpret and display the messages.

CHANGES
01-29-2013 gpc -  Created
02-26-2014 mn  -  Updated for interrupt driven IO.
03-12-2014 mn  -  Updated to use uCOS-III
*/

#include "includes.h"
#include "BfrPair.h"
#include "Reply.h"
#include "Payload.h"
#include "Error.h"
#include "PktParser.h"
#include "SerIODriver.h"
#include "Intrpt.h"
#include "assert.h"

/*----- c o n s t a n t    d e f i n i t i o n s -----*/

// Define RS232 baud rate.
#define Init_STK_SIZE 128 // Init stack size
#define InitPRIO 2 // Init task priority
#define BaudRate 9600 // Baud rate setting

/*----- f u n c t i o n    p r o t o t y p e s -----*/

void AppMain(void);

/*--------------- m a i n ( ) -----------------*/

CPU_INT32S main()
{
//  Initialize the STM32F107 eval. board.
    BSP_IntDisAll();            /* Disable all interrupts. */

    BSP_Init();                 /* Initialize BSP functions */

    BSP_Ser_Init(BaudRate);     /* Initialize the RS232 interface. */

//  Run the application.    
    AppMain();
    
    return 0;
}

/*--------------- A p p M a i n ( ) ---------------

PURPOSE
This is the application main program.

*/

void AppMain(void)
{
  BfrPair *payloadBfrPair;  // Address of the Payload Buffer Pair
  BfrPair *replyBfrPair;    // Address of the Reply Buffer Pair

  // Create and Initialize iBfrPair and oBfrPair.
  InitSerIO();
  
  // Enable interrupts globally.
  IntEn();

  // Create and initialize the Payload Buffer Pair and the Reply Buffer
  // Pair and get their addresses.
  PayloadInit(&payloadBfrPair, &replyBfrPair);
  
  // Multitasking Executive Loop: Tasks are executed round robin.
  for (;;)
    {
    // Service the RS232 receiver.
//    ServiceRx();

    // Execute the ParsePkt task.
    ParsePkt(payloadBfrPair);
 
    // Execute the Payload task.
    PayloadTask();

    // Execute the Reply Task.
    Reply(replyBfrPair);

    // Service the RS232 transmitter.
//    ServiceTx();
    }
}
