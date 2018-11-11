#ifndef USERDATA_H_
#define USERDATA_H_

#include <stdint.h>

#define FACTORY_SETTING_ADDRESS	0x000
#define SYSTEM_STATE_ADDRESS	0x001
#define USERS_ADDRESS			0x002
#define ADMINS_ADDRESS			0x003


#define FACTORY_FIRST_TIME_USE	0xFF
#define FACTORY_CONFIGURED 		0x00

#define SUCCESS 				1
#define FAIL 					0
#define NULL					0
#define NOT_FOUND				0

#define VALID    			    0
#define CORRUPTED 				1
#define EMPTY 					0xFF

#define UNLOCKED 				0
#define LOCKED 					1

#define ACTIVE 					0
#define BLOCKED					1

#define DEFAULT_ADMIN_ID		"1234"
#define DEFAULT_ADMIN_PW		"0000"


typedef enum {member = 1, moderator, admin} Rank;

uint8_t E2prom_SystemSetup(void);
uint8_t E2prom_AddUser(uint8_t * user_id, uint8_t * user_name, uint8_t * user_password, Rank rank);
uint8_t E2prom_RemoveUser(uint8_t * user_id, uint8_t * loggedIn_id);
uint8_t E2prom_ModifyUser(uint8_t * user_id, uint8_t * user_name, uint8_t * user_pw);

uint16_t CheckUserID(uint8_t * user_id);

extern uint8_t E2prom_GetUserName(uint8_t * user_id, uint8_t * user_name);
extern uint8_t E2prom_GetUserPassword(uint8_t * user_id, uint8_t * user_pw);
extern uint8_t E2prom_GetUserRank(uint8_t * user_id, Rank * user_rank);

extern uint8_t E2prom_GetSystemFactorySetting(void);
extern void E2prom_SetSystemFactorySetting(uint8_t setting);


void E2prom_ListUsers(void);
uint8_t E2prom_PromoteUser(uint8_t * user_id, Rank user_rank);

uint8_t* E2prom_VerifyUserInfo(uint8_t * user_id, uint8_t * user_pw);
Rank E2prom_VerifyAdminInfo(uint8_t * user_id, uint8_t* user_pw);


#endif


