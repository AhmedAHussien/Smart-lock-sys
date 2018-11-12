#include "i2c.h"
#include "userdata.h"
#include <string.h>
#include "uart.h"

#define USER_STATE_OFFSET		0
#define USER_STATUS_OFFSET		1
#define USER_RANK_OFFSET		2
#define USER_ID_OFFSET			3
#define USER_PASSWORD_OFFSET	8
#define USER_NAME_OFFSET		13
#define USER_CHECKSUM_OFFSET	30

static uint8_t SetUsersNumber(uint8_t users_number);
static uint8_t SetAdminsNumber(uint8_t admins_number);
static uint8_t GetUsersNumber(void);
static uint8_t GetAdminsNumber(void);
static void FactoryReset(void);
static void SystemDataSort(void);
static uint8_t RelocateUser(uint16_t source, uint16_t destination);
static uint16_t CheckUserID(uint8_t * user_id);


uint8_t g_users_number = 0;
uint8_t g_admins_number = 0;



extern void E2prom_SystemSetup(void)
{

	if(E2prom_GetSystemFactorySetting() != FACTORY_CONFIGURED)
	{
		//clear memory to make sure everything will be good
		FactoryReset();
		//unlock system by default
		SetSystemState(UNLOCKED);
		//set users number to zero
		SetUsersNumber(0);
		//set admins number to zero
		SetAdminsNumber(0);
	}
	else
	{
		//initialize system by sorting data in case power outtage during write
		//operations on the i2cE2prom
		SystemDataSort();
	}

}

extern uint8_t E2prom_AddUser(uint8_t * user_id, uint8_t * user_name, uint8_t * user_pw, Rank user_rank)
{
	uint8_t users_number = GetUsersNumber();
	uint8_t admins_number = GetAdminsNumber();
	//check for duplicated user ids'
	if(CheckUserID(user_id) != NOT_FOUND)
	{
		return FAIL;	//id already exists
	}
	else if (users_number >= MAX_NUMBER_OF_USERS)
	{
		return FAIL;
	}
	else if((user_rank == admin) && (admins_number >= MAX_NUMBER_OF_ADMINS))
	{
    return FAIL;
	}
	else
		{;}

	uint16_t target_address = 0x020 + (users_number * 0x20);
	uint8_t i= 0, j=0;
	//transmit : status, rank, id, pw, name : calculate checksum, transmit checksum
	uint16_t checksum = 0;
	checksum += (ACTIVE + user_rank);
	I2C_StartTransmit(target_address + USER_STATUS_OFFSET);
	I2C_Transmit(ACTIVE);
	I2C_Transmit(user_rank);
	for(i=0; i<5; i++)
	{
		checksum += user_id[i];
		I2C_Transmit(user_id[i]);
	}
	for(i=0; i<5; i++)
	{
		checksum += user_pw[i];
		I2C_Transmit(user_pw[i]);
	}

	for(i=0; i<17; i++)
	{
		checksum += user_name[i];
		I2C_Transmit(user_name[i]);
		if(user_name[i] == '\0')
		{
			break;
		}
	}
	for(j = i+1; j < 17; j++)
	{
		I2C_Transmit('\0');
	}

	checksum ^= 0xFFFF;
	I2C_Transmit(checksum & 0x00FF);
	I2C_Transmit(checksum >> 8);
	I2C_StopTransmit();

	//receive and compare checksum
	checksum = 0;
	I2C_StartReceive(target_address+1);
	for(i=0; i<0x1D; i++)
	{
		checksum += I2C_ReceiveAck();
	}
        checksum += I2C_ReceiveAck();
        checksum += ((uint16_t)I2C_ReceiveAck() << 8);

	I2C_StopReceive();


	if(checksum == 0xFFFF)
	{
		I2C_StartTransmit(target_address);
		I2C_Transmit(VALID);
		I2C_StopTransmit();

		users_number++;

		if(user_rank == admin)
		{
			admins_number++;
		}
		else
			{;}

		SetUsersNumber(users_number);
		SetAdminsNumber(admins_number);


		return SUCCESS;
	}
	else
	{
		SystemDataSort();
		return FAIL;
	}
}

extern uint8_t E2prom_RemoveUser(uint8_t * user_id, uint8_t * loggedIn_id)		//needs attention
{
	if(strcmp(user_id, loggedIn_id) == 0)
	{
		return FAIL;
	}
	else {;}

	uint16_t target_address = CheckUserID(user_id);
	if(target_address == 0)
	{
		return FAIL;
	}
	else {;}


	I2C_StartTransmit(target_address);
	I2C_Transmit(EMPTY);
	I2C_StopTransmit();

	SystemDataSort();


	return SUCCESS;
}

extern uint8_t E2prom_ModifyUser(uint8_t * user_id, uint8_t * user_name, uint8_t * user_pw)		//needs attention
{
	uint16_t target_address = CheckUserID(user_id);
	if(target_address == 0)
	{
		return FAIL;
	}
	else{;}


	uint8_t user_data[29] = {0}, i = 0;
	I2C_StartReceive(target_address + 1);
	for(i=0; i<29; i++)
	{
		user_data[i] = I2C_ReceiveAck();
	}
	I2C_StopReceive();


	if((*user_pw) != NULL)
	{
		for(i=0; i<5; i++)
		{
			user_data[i+7] = user_pw[i];
		}
	}
	else {;}

	if((*user_name) != NULL)
	{
		for(i=0; i<17; i++)
		{
			user_data[i+12] = user_name[i];
			if(user_name[i] == '\0')
				{
					break;
				}
		}

		for(i = i+1; i < 17; i++)
		{
			user_data[i+12] = '\0';
		}
	}
	else {;}

	uint16_t checksum = 0;
	I2C_StartTransmit(target_address);
	I2C_Transmit(CORRUPTED);
	for(i=0; i<29; i++)
	{
		checksum += user_data[i];
		I2C_Transmit(user_data[i]);
	}
	checksum ^= 0xFFFF;
	I2C_Transmit(checksum & 0x00FF);
	I2C_Transmit(checksum >> 8);
	I2C_StopTransmit();

	checksum = 0;
	I2C_StartReceive(target_address+1);
	for(i=0; i<29; i++)
	{
		checksum += I2C_ReceiveAck();
	}
        checksum += I2C_ReceiveAck();
        checksum += ((uint16_t)I2C_ReceiveAck() << 8);

	I2C_StopReceive();

	if(checksum == 0xFFFF)
	{
		I2C_StartTransmit(target_address);
		I2C_Transmit(VALID);
		I2C_StopTransmit();
		return SUCCESS;
	}
	else
	{
		SystemDataSort();
		return FAIL;
	}
}

extern void E2prom_ListUsers(void)		//Check
{

	uint8_t users_number = GetUsersNumber();

	uint8_t i = 0, j = 0, user_iterator = 1;
	uint8_t user_name[17];
	uint8_t user_id[5];
	Rank user_rank;
	for(i=1; i<=users_number; i++)
	{
		I2C_StartReceive(i*(0x020) + USER_STATE_OFFSET);
		uint8_t user_state = I2C_ReceiveAck();
		I2C_StopReceive();

		if(user_state == VALID)
		{
			I2C_StartReceive(i*0x020 + USER_RANK_OFFSET);
			user_rank = (Rank)I2C_ReceiveAck();
			I2C_StopReceive();

			I2C_StartReceive(i*(0x020) + USER_ID_OFFSET);
			for(j=0; j<5; j++)
			{
				user_id[j] = I2C_ReceiveAck();
			}
			I2C_StopReceive();

			I2C_StartReceive(i*(0x020) + USER_NAME_OFFSET);
			for(j=0; j<17; j++)
			{
				user_name[j] = I2C_ReceiveAck();
				if(user_name[j] == '\0')
				{
					break;
				}
				else
					{;}
			}
			I2C_StopReceive();

			if(user_iterator <= 9)
			{
				UART_SendChar(user_iterator + 0x30);
			}
			else if (user_iterator > 9 && user_iterator <= 99)
			{
				UART_SendChar(user_iterator / 10);
				UART_SendChar(user_iterator % 10);
			}

			else if(user_iterator > 99 && user_iterator <= 255)
			{
				UART_SendChar(user_iterator / 100);
				UART_SendChar((user_iterator % 100) / 10);
				UART_SendChar(user_iterator % 10);
			}
			else
				{;}


			UART_SendString(". ");
			UART_SendString(user_name);
			UART_SendChar('\t');
			UART_SendString(user_id);
			UART_SendChar('\t');
			switch(user_rank)
			{
				case member:
					UART_SendString("Member\r\n");
				break;

				case moderator:
					UART_SendString("Moderator\r\n");
				break;

				case admin:
					UART_SendString("Adminstrator\r\n");
				break;

				default:
				break;
			}
			user_iterator++;
		}
	}
}

extern Rank E2prom_VerifyAdminInfo(uint8_t * user_id, uint8_t* user_pw)		//needs attention
{
	uint16_t user_address = CheckUserID(user_id);
	Rank user_rank;
	if(user_address == 0)
	{
		return FAIL;
	}
	else
	{
		I2C_StartReceive(user_address);
		uint8_t user_state = I2C_ReceiveAck();
		I2C_StopReceive();
		if(user_state != VALID)
		{
			return FAIL;
		}
		else
		{
			I2C_StartReceive(user_address + 2);
			user_rank = I2C_ReceiveAck();
			I2C_StopReceive();
			if(user_rank == member)
			{
				return FAIL;
			}
			else
				{;}
		}
	}

	uint8_t i = 0, user_received_pw[5];
	I2C_StartReceive(user_address + 8);
	for(i=0; i<5; i++)
	{
		user_received_pw[i] = I2C_ReceiveAck();
	}
	I2C_StopReceive();
	if(strcmp(user_received_pw, user_pw) == 0)
	{
		return user_rank;
	}
	else
	{
		return FAIL;
	}
}

extern uint8_t* E2prom_VerifyUserInfo(uint8_t * user_id, uint8_t * user_pw)		//needs attention
{
	uint16_t user_address = CheckUserID(user_id);
	if(user_address == 0)
	{
		return FAIL;
	}
	else
	{
		I2C_StartReceive(user_address);
		uint8_t user_state = I2C_ReceiveAck();
		I2C_StopReceive();
		if(user_state != VALID)
		{
			return FAIL;
		}
		else
		{
			uint8_t i = 0, user_received_pw[5];
			I2C_StartReceive(user_address + 8);
			for(i=0; i<5; i++)
			{
				user_received_pw[i] = I2C_ReceiveAck();
			}
			I2C_StopReceive();
			if(strcmp(user_received_pw, user_pw) == 0)
			{
				static uint8_t user_name[17];
				I2C_StartReceive(user_address+13);
				for(i=0; i<17; i++)
				{
					user_name[i] = I2C_ReceiveAck();
					if (user_name[i] == '\0')
					{
						break;
					}
				}
                                I2C_StopReceive();
				return user_name;
			}
			else
			{
				return FAIL;
			}
		}
	}
}



/********************** Getters *************************/

extern uint8_t E2prom_GetUserName(uint8_t * user_id, uint8_t * user_name)
{
	uint16_t user_address = CheckUserID(user_id);
	if(user_address == NOT_FOUND)
	{
		return NOT_FOUND;
	}
	else
	{
		uint8_t i = 0;
		for(i=0; i<17; i++)
		{
			I2C_StartReceive(user_address + USER_NAME_OFFSET);
			user_name[i] = I2C_ReceiveAck();
			if(user_name[i] == '\0')
			{
				I2C_StopReceive();
				break;
			}
			else
				{;}
		}

		return SUCCESS;
	}

}

extern uint8_t E2prom_GetUserPassword(uint8_t * user_id, uint8_t * user_pw)
{
	uint16_t user_address = CheckUserID(user_id);
	if(user_address == NOT_FOUND)
	{
		return NOT_FOUND;
	}
	else
	{
		uint8_t i = 0;
		for(i=0; i<4; i++)
		{
			I2C_StartReceive(user_address + USER_PASSWORD_OFFSET);
			user_pw[i] = I2C_ReceiveAck();
		}
		I2C_StopReceive();
		return SUCCESS;
	}
}

extern uint8_t E2prom_GetUserRank(uint8_t * user_id, Rank * user_rank)
{
	uint16_t user_address = CheckUserID(user_id);
	if(user_address == NOT_FOUND)
	{
		return NOT_FOUND;
	}
	else
	{
		I2C_StartReceive(user_address + USER_RANK_OFFSET);
		*user_rank = I2C_ReceiveAck();
		I2C_StopReceive();

		return SUCCESS;
	}
}

extern uint8_t E2prom_GetSystemFactorySetting(void)			//check
{
	uint8_t sys_factory_setting = 0;

	I2C_StartReceive(FACTORY_SETTING_ADDRESS);
	sys_factory_setting = I2C_ReceiveAck();
	I2C_StopReceive();

	return sys_factory_setting;
}



/********************** Setters *************************/

extern uint8_t E2prom_SetSystemFactorySetting(uint8_t setting)		//check
{
	if(setting == FACTORY_FIRST_TIME_USE || setting == FACTORY_CONFIGURED)
	{
		I2C_StartTransmit(FACTORY_SETTING_ADDRESS);
		I2C_Transmit(setting);
		I2C_StopTransmit();
		return SUCCESS;
	}
	else
	{
		return FAIL;
	}

}

extern uint8_t SetSystemState(uint8_t system_state)		//check
{
	if(system_state == UNLOCKED || system_state == LOCKED)
	{
		I2C_StartTransmit(SYSTEM_STATE_ADDRESS);
		I2C_Transmit(system_state);
		I2C_StopTransmit();

		return SUCCESS;
	}
	else
	{
		return FAIL;
	}

}

/******************* Static Utils **********************/

static uint8_t SetUsersNumber(uint8_t users_number)		//check
{
	if(users_number <= 126)
	{
		I2C_StartTransmit(USERS_ADDRESS);
		I2C_Transmit(users_number);
		I2C_StopTransmit();
		return SUCCESS;
	}
	else
	{
		return FAIL;
	}
}

static uint8_t SetAdminsNumber(uint8_t admins_number)		//check
{
	if(admins_number <= MAX_NUMBER_OF_ADMINS)
	{
		I2C_StartTransmit(ADMINS_ADDRESS);
		I2C_Transmit(admins_number);
		I2C_StopTransmit();
		return SUCCESS;
	}
	else
	{
		return FAIL;
	}
}

static uint8_t GetUsersNumber(void)		//check
{
	uint8_t users_number = 0;
	I2C_StartReceive(USERS_ADDRESS);
	users_number = I2C_ReceiveAck();
	I2C_StopReceive();

	return users_number;
}

static uint8_t GetAdminsNumber(void)		//check
{
	uint8_t admins_number = 0;
	I2C_StartReceive(ADMINS_ADDRESS);
	admins_number = I2C_ReceiveAck();
	I2C_StopReceive();

	return admins_number;
}

static void FactoryReset(void)		//check
{
  uint16_t i=0;
  uint8_t j =0;

  for(i=0; i<0x128; i++)
  {
  	I2C_StartTransmit(i*0x020);
  	for(j=0; j<0x20; j++)
  	{
  		I2C_Transmit(0xFF);
  	}
  	I2C_StopTransmit();
  }
}

extern uint16_t CheckUserID(uint8_t * user_id)		//check
{

	uint8_t users_number = GetUsersNumber();

	uint8_t i = 0, j = 0, id_cmp[5];

	for(i=1; i<= users_number; i++)
	{
		I2C_StartReceive(i*0x020 + USER_ID_OFFSET);
		for(j=0; j<5; j++)
		{
			id_cmp[j] = I2C_ReceiveAck();
		}
		I2C_StopReceive();
		if(strcmp(user_id, id_cmp) == 0)
		{
			return (i*0x20); //id exists and returns the address of the user
		}
	}

	return NOT_FOUND;

}

static void SystemDataSort(void)		//check
{
	//read number of users in global variable
	uint8_t users_number = GetUsersNumber();
	uint8_t admins_number = 0;
	//loop for number of users
	uint8_t i = 0, j = 0, user_state;
	for(i=1; i<=users_number; i++)
	{
		I2C_StartReceive(i*0x020);
		user_state = I2C_ReceiveAck();
		I2C_StopReceive();
		if(user_state != VALID)
		{
			for(j=users_number; j>=i; j--)
			{
				I2C_StartReceive(j*0x020);
				user_state = I2C_ReceiveAck();
				I2C_StopReceive();
				if(user_state != VALID)
				{
					users_number--;
				}
				else
				{
					RelocateUser(j*0x020, i*0x020);
					users_number--;
					I2C_StartTransmit(j*0x020);
					I2C_Transmit(EMPTY);
					I2C_StopTransmit();
					break;
				}
			}
		}
		else
		{;}

		if(user_state == VALID)
		{
			I2C_StartReceive(i*0x020 + USER_RANK_OFFSET);
			if(I2C_ReceiveAck() == admin)
			{
				admins_number++;
			}
			I2C_StopReceive();
		}
		else
			{;}

	}

	SetUsersNumber(users_number);
	SetAdminsNumber(admins_number);

}

static uint8_t RelocateUser(uint16_t source, uint16_t destination)		//check
{
	uint8_t user_data[31], i=0;
	I2C_StartReceive(source + 1);
	for(i=0; i<31; i++)
	{
		user_data[i] = I2C_ReceiveAck();
	}
	I2C_StopReceive();

	I2C_StartTransmit(destination + 1);
	for(i=0; i<31; i++)
	{
		I2C_Transmit(user_data[i]);
	}
	I2C_StopTransmit();

	I2C_StartTransmit(destination);
	I2C_Transmit(VALID);
	I2C_StopTransmit();
	return SUCCESS;

}
