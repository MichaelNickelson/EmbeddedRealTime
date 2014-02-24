/*--------------- E r r o r . h ---------------

by: Michael Nickelson

PURPOSE
Generate error and assert messages
Header file

CHANGES
02-19-14 mn -  Initial submission
03-12-14 mn -  Remove preamble error prototype as the function is no longer used
*/

#ifndef Errors_H
#define Errors_H

/* Error types used when calling error display functions */
typedef enum {ERR_PREAMBLE_1 = -1,
              ERR_PREAMBLE_2 = -2,
              ERR_PREAMBLE_3 = -3,
              ERR_CHECKSUM = -4, 
              ERR_LEN = -5,
              ERR_MSG_TYPE = -6} Error_t;

typedef enum {ASS_ADDRESS} Assert_t;

/*----- f u n c t i o n    p r o t o t y p e s -----*/
void DispErr(Error_t e, CPU_CHAR reply[]);
void DispAssert(Assert_t a, CPU_CHAR reply[]);

#endif