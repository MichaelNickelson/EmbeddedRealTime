#ifndef Errors_H
#define Errors_H
/*---------------------------- FILE: Errors.h ----------------------------*/

typedef enum {ERR_CHECKSUM,
              ERR_LEN,
              ERR_MESSAGE_TYPE,
              ERR_UNKNOWN} Error_t;

typedef enum {ASS_ADDRESS} Assert_t;

void DispErr(Error_t e);

void DispAssert(Assert_t a);

void PreambleError(CPU_INT08U pe);

#endif