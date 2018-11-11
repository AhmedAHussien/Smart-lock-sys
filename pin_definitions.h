
#ifndef _PIN_DEFINITIONS_H_
#define _PIN_DEFINITIONS_H_


/* ================================================================================ */
/* ================                     SYSCTL                     ================ */
/* ================================================================================ */

#define R0      0
#define R1      1
#define R2      2
#define R3      3
#define R4      4
#define R5      5

//**********************************************************************************//



/* ================================================================================ */
/* ================                      I2C                       ================ */
/* ================================================================================ */

//*****************         I2C Master Configuration (I2CMCR)      *****************//
#define GFE     6
#define SFE     5
#define MFE     4
#define LPBK    0

//*****************         I2C Master Timer Period (I2CMTPR)      *****************//
#define HS      7
#define TPRate     0

//*****************         I2C Master Control/Status (I2CMCS)     *****************//
#define CLTO    7
#define BUSBSY  6
#define IDLE    5
#define ARBLST  4
#define DATACK  3
#define ADRACK  2
#define ERROR   1
#define BUSY    0

#define HS_MCS      4
#define ACK     3
#define STOP    2
#define START   1
#define RUN     0

//**********************************************************************************//




#endif