#ifndef USERDATA_H_
#define USERDATA_H_

#include <stdint.h>

#define FACTORY_SETTING_ADDRESS   0x000
#define SYSTEM_STATE_ADDRESS      0x001
#define USERS_ADDRESS             0x002
#define ADMINS_ADDRESS            0x003


#define FACTORY_CONFIGURED        1
#define FACTORY_FIRST_TIME_USE    0xFF

#define SUCCESS                   1
#define FAIL                      0
#define NULL                      0
#define NOT_FOUND                 0

#define VALID                     1
#define CORRUPTED                 0
#define EMPTY                     0xFF

#define UNLOCKED                  1
#define LOCKED                    0xFF

#define ACTIVE                    1
#define BLOCKED                   0xFF

#define DEFAULT_ADMIN_ID		      "1234"
#define DEFAULT_ADMIN_PW		      "0000"

#define MAX_NUMBER_OF_ADMINS      2
#define MAX_NUMBER_OF_USERS       127


typedef enum {member = 1, moderator, admin} Rank;

void E2prom_SystemSetup(void);
uint8_t E2prom_AddUser(uint8_t * user_id, uint8_t * user_name, uint8_t * user_password, Rank rank);
uint8_t E2prom_RemoveUser(uint8_t * user_id);
uint8_t E2prom_ModifyUser(uint8_t * user_id, uint8_t * user_name, uint8_t * user_pw);

uint16_t CheckUserID(uint8_t * user_id);

/*************** Getters **************/
extern uint8_t E2prom_GetUserName(uint8_t * user_id, uint8_t * user_name);
extern uint8_t E2prom_GetUserPassword(uint8_t * user_id, uint8_t * user_pw);
extern uint8_t E2prom_GetUserRank(uint8_t * user_id, Rank * user_rank);
extern uint8_t E2prom_GetSystemFactorySetting(void);
extern uint8_t E2prom_GetUserStatus(uint8_t * user_id, uint8_t * user_status);

/*************** Setters *************/
extern uint8_t E2prom_SetSystemFactorySetting(uint8_t setting);
extern uint8_t SetSystemState(uint8_t system_state);


void E2prom_ListUsers(void);
uint8_t E2prom_PromoteUser(uint8_t * user_id, Rank user_rank);

uint8_t E2prom_VerifyUserInfo(uint8_t * user_id, uint8_t * user_pw);


#endif
