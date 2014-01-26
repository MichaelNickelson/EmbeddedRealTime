#ifndef Errors_H
#define Errors_H
/*---------------------------- FILE: Errors.h ----------------------------*/

typedef enum {ERR_PRE1,
              ERR_PRE2,
              ERR_PRE3,
              ERR_CHECKSUM,
              ERR_LEN,
              ERR_ADDRESS,
              ERR_MESSAGE_TYPE} Error_t;

void DispErr(Error_t err);

#endif