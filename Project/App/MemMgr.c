/*=============== M e m M g r . c ===============*/

/*
BY:	George Cheney
		16.472 / 16.572 Embedded Real Time Systems
		Electrical and Computer Engineering Dept.
		UMASS Lowell

PURPOSE
Allocate and deallocate buffers from a pool.

DEMONSTRATES
Memory Management
Buffer Pool

CHANGES
03-24-2011  - Release to class.
04-30-2014 - mn - Slight modifications to work with buffer implementation
*/

#include "includes.h"
#include "assert.h"
//#include "Globals.h"
#include "MemMgr.h"

/*----- c o n s t a n t    d e f i n i t i o n s -----*/

#define SuspendTimeout 100	/* -- Timeout suspended task if not awakened. */

/*----- g l o b a l s -----*/

/* Space for "PoolSize" buffers */
Buffer	bfrSpace[PoolSize];

/* Memory partition control block to manage the buffer pool */
OS_MEM bfrPool;

/* Semaphores */
OS_SEM bfrAvail;	/* Number of free buffers remaining in the pool */


/*--------------- I n i t M e m M g r ( ) ---------------*/

/*
PURPOSE
Initialize the memory manager.

GLOBALS
bfrPool		  -- Pool of free buffers
bfrSpace    -- Memory space for pool buffers
bfrAvail		-- Semaphore to allow buffer requests when pool is not empty
*/
void InitMemMgr(void)
{
  OS_ERR osErr;/* -- uCos Error Code */   
   
  /* Create and initialize semaphore. */
  OSSemCreate(&bfrAvail, "Buffer Available", PoolSize, &osErr);
  assert(osErr == OS_ERR_NONE);

  /* Create a pool of "PoolSize" buffers. */
  OSMemCreate(&bfrPool, "Buffer Pool", bfrSpace, PoolSize, sizeof(Buffer), &osErr);
  assert(osErr == OS_ERR_NONE);
}

/*--------------- A l l o c a t e ( ) ---------------*/

/*
PURPOSE
Allocate a buffer from the buffer pool - block if none available.

GLOBALS
bfrPool  -- Pool of free buffers
bfrAvail -- Semaphore to allow buffer requests when pool is not empty

RETURNS
The address of the allocated buffer.
*/
Buffer *Allocate(void)
{
  OS_ERR osErr;/* -- uCos Error Code */   
   
  Buffer *bfr;  // Allocated buffer address
//  CPU_INT08U bfrSpace[24];
  CPU_INT16U size = 24;
   
  /* Wait until there is an available buffer. */
  OSSemPend(&bfrAvail, SuspendTimeout, OS_OPT_PEND_BLOCKING, NULL, &osErr);
  assert(osErr==OS_ERR_NONE);

  /* Get the buffer from the pool. */
  bfr = (Buffer*) OSMemGet(&bfrPool, &osErr);
  assert(osErr==OS_ERR_NONE);
	
  /* Initialize the buffer to prepare for filling. */
  BfrInit(bfr, size);
//  BfrReset(bfr);
	
  return bfr;
}   	

/*--------------- F r e e ( ) ---------------*/

/*
PURPOSE
Return a buffer to the pool.

INPUT PARAMETERS
bfr   -- the address of the buffer to be returned

GLOBALS
bfrPool		  -- Pool of free buffers
bfrAvail		-- Semaphore to allow buffer requests when pool is not empty
*/
void Free(Buffer *bfr)
{
	OS_ERR		osErr;/* -- uCos Error Code */   

   /* Return the buffer. */
	OSMemPut(&bfrPool, bfr, &osErr);
	assert(osErr==OS_ERR_NONE);
	
	/* Indicate that one more buffer is available. */
  OSSemPost(&bfrAvail, OS_OPT_POST_1, &osErr);
  assert(osErr==OS_ERR_NONE);
}   