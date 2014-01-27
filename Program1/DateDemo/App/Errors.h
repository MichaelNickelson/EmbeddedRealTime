#ifndef Errors_H
#define Errors_H
/*---------------------------- FILE: Errors.h ----------------------------*/

typedef enum {ERR_PRE1,
              ERR_PRE2,
              ERR_PRE3,
              ERR_CHECKSUM,
              ERR_LEN,
              ERR_MESSAGE_TYPE} Error_t;

typedef enum {ASS_ADDRESS} Assert_t;

void DispErr(Error_t e);

void DispAssert(Assert_t a);

#endif