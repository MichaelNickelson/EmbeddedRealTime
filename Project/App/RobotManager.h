/*--------------- R o b o t M a n a g e r . h ---------------

by: Michael Nickelson

PURPOSE - Header file
Handle robot packets and responses

CHANGES
04-30-2014 mn -  Initial submission
*/

#ifndef ROBOT_H
#define ROBOT_H

/*-----  Assign easy to read names to message ID -----*/
// In the header to be used by Error.c, Framer.c, and RobotControl.c
#define MSG_RESET 0x00
#define MSG_ADD 0x01
#define MSG_MOVE 0x02
#define MSG_PATH 0x03
#define MSG_LOOP 0x04
#define MSG_STOP 0x05
#define MSG_STEP 0x07
#define MSG_HERE_I_AM 0x09
#define MSG_ACK 0x0A
#define MSG_ERR 0x0B

/*----- f u n c t i o n    p r o t o t y p e s -----*/
void CreateRobotMgrTask(void);

#endif