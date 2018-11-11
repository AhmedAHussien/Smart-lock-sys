#ifndef _TERMINAL_H_
#define _TERMINAL_H_

#include <stdint.h>
#define CANCELLED 0
#define SUCCESS 1
#include "userdata.h"


extern uint8_t Terminal_ReceiveUserID(uint8_t * user_id);
extern uint8_t Terminal_ReceiveUserPassword(uint8_t * user_password);
extern uint8_t Terminal_ReceiveUserName(uint8_t * user_name);
extern uint8_t Terminal_ReceiveUserRank(Rank * user_rank);

#endif