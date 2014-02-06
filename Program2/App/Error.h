#ifndef Errors_H
#define Errors_H
/*---------------------------- FILE: Errors.h ----------------------------*/

typedef enum {ERR_PREAMBLE_1 = -1,
              ERR_PREAMBLE_2 = -2,
              ERR_PREAMBLE_3 = -3,
              ERR_CHECKSUM = -4, 
              ERR_LEN = -5,
              ERR_MSG_TYPE = -6} Error_t;

typedef enum {ASS_ADDRESS} Assert_t;

void DispErr(Error_t e, CPU_CHAR reply[]);

void DispAssert(Assert_t a, CPU_CHAR reply[]);

void PreambleError(CPU_INT08U pe);

#endif