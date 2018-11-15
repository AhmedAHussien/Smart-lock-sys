#ifndef USERDATA_H_
#define USERDATA_H_

#include <stdint.h>

#define FACTORY_SETTING_ADDRESS       0x000
#define SYSTEM_STATE_ADDRESS          0x001
#define USERS_ADDRESS                 0x002
#define ADMINS_ADDRESS                0x003
#define INCORRECT_PW_COUNTER_ADDRESS  0x004
#define GATE_STATE_ADDRESS            0x005


#define FACTORY_CONFIGURED            1
#define FACTORY_FIRST_TIME_USE        0xFF

#define SUCCESS                       1
#define FAIL                          0
#define NULL                          0
#define NOT_FOUND                     0

#define VALID                         1
#define CORRUPTED                     0
#define EMPTY                         0xFF

#define UNLOCKED                      1
#define LOCKED                        0xFF

#define OPENED                        1
#define CLOSED                        2
#define OVERRIDEN                     3

#define ACTIVE                        1
#define BLOCKED                       0xFF

#define DEFAULT_ADMIN_ID		          "1234"
#define DEFAULT_ADMIN_PW		          "0000"

#define MAX_NUMBER_OF_ADMINS          2
#define MAX_NUMBER_OF_USERS           127
#define MAX_WRONG_ENTRIES             3


typedef enum {member = 1, moderator, admin} Rank;

extern void E2prom_SystemSetup(void);
extern uint8_t E2prom_AddUser(uint8_t * user_id, uint8_t * user_name, uint8_t * user_password, Rank rank);
extern uint8_t E2prom_RemoveUser(uint8_t * user_id);
extern uint8_t E2prom_ModifyUser(uint8_t * user_id, uint8_t * user_name, uint8_t * user_pw, Rank user_rank);
extern uint8_t E2prom_VerifyUserInfo(uint8_t * user_id, uint8_t * user_pw);
extern void E2prom_ListUsers(void);

/*************** Getters **************/
extern uint8_t E2prom_GetUserName(uint8_t * user_id, uint8_t * user_name);
extern uint8_t E2prom_GetUserPassword(uint8_t * user_id, uint8_t * user_pw);
extern uint8_t E2prom_GetUserRank(uint8_t * user_id, Rank * user_rank);
extern uint8_t E2prom_GetSystemFactorySetting(void);
extern uint8_t E2prom_GetUserStatus(uint8_t * user_id, uint8_t * user_status);
extern uint8_t E2prom_GetSystemState(void);
extern uint8_t E2prom_GetIncorrectPWCounter(void);
extern uint8_t E2prom_GetGateState(void);

/*************** Setters *************/
extern uint8_t E2prom_IncrementIncorrectPWCounter(void);
extern void E2prom_ResetIncorrectPWCounter(void);
extern uint8_t E2prom_SetSystemFactorySetting(uint8_t setting);
extern uint8_t E2prom_SetSystemState(uint8_t system_state);
extern void E2prom_SetGateState(uint8_t door_state);




#endif
