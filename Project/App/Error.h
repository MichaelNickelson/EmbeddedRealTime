/*--------------- E r r o r . h ---------------

by: Michael Nickelson

PURPOSE
Generate error and assert messages
Header file

CHANGES
02-19-2014 mn -  Initial submission
03-12-2014 mn -  Remove preamble error prototype as the function is no longer used
04-30-2014 mn -  Add errors needed for final project
*/

#ifndef Errors_H
#define Errors_H

#include "Buffer.h"
#include "RobotMgr.h"

#define ERROR_MULTIPLIER 10

#define ROBOT_ADDRESS 1
#define NON_EXISTENT_ROBOT 2
#define BAD_LOCATION 3

/* Error types used when calling error display functions */
typedef enum {ERR_PREAMBLE_1 = 1,
              ERR_PREAMBLE_2 = 2,
              ERR_PREAMBLE_3 = 3,
              ERR_CHECKSUM = 4, 
              ERR_LEN = 5,
              
              ERR_ADD_ADDRESS = 11,
              ERR_ADD_LOC = 12,
              ERR_ADD_OCCUPIED = 13,
              ERR_ADD_EXISTS = 14,
              
              ERR_MOVE_ADDRESS = 21,
              ERR_MOVE_NON_EXIST = 22,
              ERR_MOVE_LOC = 23,
              
              ERR_FOL_ADDRESS = 31,
              ERR_FOL_NON_EXIST = 32,
              ERR_FOL_LOC = 33,
              
              ERR_LOOP_ADDRESS = 41,
              ERR_LOOP_NON_EXIST = 42,
              ERR_LOOP_LOC = 43,
              
              ERR_STOP_ADDRESS = 51,
              ERR_STOP_NON_EXIST = 52,
              
              ERR_MGR_TYPE = 61
              } Error_t;

/*----- f u n c t i o n    p r o t o t y p e s -----*/
void SendError(Error_t e);

#endif