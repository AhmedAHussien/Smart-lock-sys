============ SYSTEM DATA LAYOUT ============

#define FACTORY_SETTING_ADDRESS	0x000		(0xFF >> first time use -- 0x00 >> configured)
#define SYSTEM_STATE_ADDRESS	0x001		(0xFF >> Locked -- 0x00 Unlocked)
#define USERS_ADDRESS			0x002
#define ADMINS_ADDRESS			0x003



============ USER DATA LAYOUT ============

#define USER_STATE_OFFSET		0			
#define USER_STATUS_OFFSET		1
#define USER_RANK_OFFSET		2
#define USER_ID_OFFSET			3
#define USER_PASSWORD_OFFSET	8
#define USER_NAME_OFFSET		13
#define USER_CHECKSUM_OFFSET	30



#define FACTORY_FIRST_TIME_USE	0xFF
#define FACTORY_CONFIGURED 		0x00

#define SUCCESS 			1
#define FAIL 				0
#define NULL				0
#define NOT_FOUND			0

#define VALID    		    0
#define CORRUPTED 			1
#define EMPTY 				0xFF

#define UNLOCKED 			0
#define LOCKED 				1

#define ACTIVE 				0
#define BLOCKED				1


#define USER_STATE_OFFSET		0
#define USER_STATUS_OFFSET		1
#define USER_RANK_OFFSET		2
#define USER_ID_OFFSET			3
#define USER_PASSWORD_OFFSET	8
#define USER_NAME_OFFSET		13
#define USER_CHECKSUM_OFFSET	30