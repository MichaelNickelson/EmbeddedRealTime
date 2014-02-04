#ifndef Errors_H
#define Errors_H
/*---------------------------- FILE: Errors.h ----------------------------*/

typedef enum {ERR_CHECKSUM = -4, 
              ERR_LEN = -5,
              ERR_MESSAGE_TYPE,
              ERR_UNKNOWN} Error_t;

typedef enum {ASS_ADDRESS} Assert_t;

void DispErr(Error_t e);

void DispAssert(Assert_t a);

void PreambleError(CPU_INT08U pe);

#endif