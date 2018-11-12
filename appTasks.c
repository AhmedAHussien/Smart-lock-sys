#include "appTasks.h"
#include "keypad.h"
#include "LCD.h"
#include "userdata.h"
#include "freeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "uart.h"
#include <string.h>
#include "TM4C123GH6PM.h"
#include "terminal.h"

static uint8_t g_Password[5] = {0};
static uint8_t g_ID[5] = {0};
static uint8_t passwordIncorrectCounter = 0;

xQueueHandle LCD_Queue = NULL;
xQueueHandle Terminal_Queue = NULL;
xSemaphoreHandle UartSemaphore = NULL;
extern TaskHandle_t keypadHandle;
extern TaskHandle_t lcdHandle;
extern TaskHandle_t terminalHandle;



void vTask_Keypad(void * args)
{

	uint8_t KeypadLastState = NOTPRESSED;
	uint8_t KeypadKey = 0;
	Keypad_Init();
	vTaskDelay(1000/portTICK_PERIOD_MS);
	while(1)
	{
		KeypadKey = Keypad_ReadChar();
		if(KeypadKey != NOTPRESSED && KeypadLastState == NOTPRESSED)
		{
			xQueueSend(LCD_Queue, (void *)&KeypadKey, 0);
		}
		KeypadLastState = KeypadKey;
		vTaskDelay(50/portTICK_PERIOD_MS);
	}
}

void vTask_LCD(void * args)
{

	LCD_Init();

	if(E2prom_GetSystemFactorySetting() == FACTORY_FIRST_TIME_USE)
	{
		LCD_WriteNewLine("SYSTEM IS NOT");
		LCD_GoTo(0,1);
		LCD_WriteString("CONFIGURED");
	}
	else
	{
		LCD_WriteNewLine("Initializing");
		LCD_GoTo(0,1);
		LCD_WriteString("System");
	}

	vTaskResume(terminalHandle);

	uint8_t KeypadKey = 0;
	uint8_t passwordIndex = 0;
	uint8_t idIndex = 0;
	BaseType_t queueState = pdFALSE;
	LCD_State_t LCD_CurrentState = Welcome;

	LCD_Init();
	LCD_WriteNewLine("Welcome");

	while(1)
	{
		queueState = xQueueReceive(LCD_Queue, &KeypadKey, 5000/portTICK_PERIOD_MS);

		if(queueState == pdPASS)
		{
			switch(LCD_CurrentState)
			{

			case Welcome:
				if(KeypadKey == '#')
				{
					//Clear array
					LCD_CurrentState = Receiving_ID;
					LCD_WriteNewLine("Enter ID:");
					LCD_GoTo(0,1);
				}
				break;

			case Receiving_ID:
				switch(KeypadKey)
				{
					case 'C':
						if(idIndex > 0)
						{
							idIndex --;
							LCD_WriteCommand(LCD_CURSOR_SHIFT_LEFT);
							LCD_WriteChar(' ');
							LCD_WriteCommand(LCD_CURSOR_SHIFT_LEFT);
							g_ID[idIndex] = '\0';
						}
						break;

					case '#':
						g_ID[idIndex] = '\0';
						idIndex = 0;
						LCD_CurrentState = Receiving_Password;
						LCD_WriteNewLine("Enter password:");
						LCD_GoTo(0,1);

						break;

					default:
						if(idIndex < 4 && KeypadKey >= 0x30 && KeypadKey <= 0x39)
						{
							g_ID[idIndex] = KeypadKey;
							idIndex++;
							LCD_WriteChar(KeypadKey);
						}
						break;
				}
				break;

			case Receiving_Password:
				switch(KeypadKey)
				{
					case 'C':
						if(passwordIndex > 0)
						{
							passwordIndex --;
							LCD_WriteCommand(LCD_CURSOR_SHIFT_LEFT);
							LCD_WriteChar(' ');
							LCD_WriteCommand(LCD_CURSOR_SHIFT_LEFT);
							g_Password[passwordIndex] = '\0';
						}
						break;

					case '#':
						vTaskSuspend(keypadHandle);
						g_Password[passwordIndex] = '\0';
						passwordIndex = 0;
						LCD_CurrentState = Submit;
				uint8_t* user_name = 0;
					user_name = E2prom_VerifyUserInfo(g_ID, g_Password);

				if(user_name != 0)
				{
			passwordIncorrectCounter = 0;
			LCD_WriteNewLine("Welcome");
			LCD_GoTo(0,1);
			LCD_WriteString(user_name);
			UART_SendString(user_name);
			UART_SendString(" has logged in\n\r");

														//Green LED on & open door
														//wait 2 seconds
														//stop door
														//wait 2 seconds
														//close door
														//wait 2 seconds
														//Green LED off & stop door
														//unblock keypad

							vTaskDelay(6000/portTICK_PERIOD_MS);

				}
				else
				{
								passwordIncorrectCounter++;
								UART_SendString("Wrong entry with ID: ");
								UART_SendString(g_ID);
								UART_SendString(" - PW: ");
								UART_SendString(g_Password);
								UART_SendString("\n\r");
								if(passwordIncorrectCounter == 3)
								{
									LCD_WriteNewLine("Access Denied");
									UART_SendString("3 Wrong Entries, System lock down\n\r");
									while(passwordIncorrectCounter == 3)
									{
										vTaskDelay(500/portTICK_PERIOD_MS);
										LCD_WriteCommand(LCD_DISPLAY_OFF);
										vTaskDelay(500/portTICK_PERIOD_MS);
										LCD_WriteCommand(LCD_DISPLAY_ON);
									}
								}
								else
								{
									LCD_WriteNewLine("Wrong Entires: ");
									LCD_WriteChar(passwordIncorrectCounter + 0x30);
									vTaskDelay(2500/portTICK_PERIOD_MS);
								}

				}
					vTaskResume(keypadHandle);
						break;

					default:
						if(passwordIndex<4 && KeypadKey >= 0x30 && KeypadKey <= 0x39)
						{
							g_Password[passwordIndex] = KeypadKey;
							passwordIndex++;
							LCD_WriteChar(KeypadKey);
							xQueuePeek(LCD_Queue, &KeypadKey, 1000/portTICK_PERIOD_MS);
							LCD_WriteCommand(LCD_CURSOR_SHIFT_LEFT);
							LCD_WriteChar('*');
						}
						break;
				}
				break;

			case Submit:
				LCD_CurrentState = Welcome;
				passwordIndex = 0;
				LCD_WriteNewLine("Welcome");
				break;
			}

		}
		else
		{
			if(LCD_CurrentState != Welcome)
			{
				LCD_CurrentState = Welcome;
				passwordIndex = 0;
				LCD_WriteNewLine("Welcome");
			}
		}

	}
}



void vTask_Terminal(void * args)
{
	//Suspend the terminal task until the lcd initializes and checks for factory settings
	vTaskSuspend(NULL);
	//wait until task resume from lcd task and then suspend lcd task
	vTaskSuspend(lcdHandle);
	vTaskSuspend(keypadHandle);


	Terminal_State_t Terminal_CurrentState = Idle;

	uint8_t terminalInput = 0;
	uint8_t adminID[5];
	uint8_t adminPassword[5];

	uint8_t i = 0;
	uint8_t user_password[5];
	uint8_t user_id[5];
	uint8_t user_name[17];
	Rank user_rank;

	Rank loggedInRank;


	if(E2prom_GetSystemFactorySetting() == FACTORY_FIRST_TIME_USE)
	{
    	UART_SendString("Welcome\r\nPlease set up the system for first time use\r\nEnter admin login info");
    	while(1)
    	{
	    	UART_SendString("\r\nEnter Admin ID: ");
				if(Terminal_ReceiveUserID(adminID) == CANCELLED)
				{
					UART_SendString("\r\nLogin cancelled");
				}
				else
				{
					UART_SendString("\r\nEnter password: ");
					if(Terminal_ReceiveUserPassword(adminPassword) == CANCELLED)
					{
						UART_SendString("\r\nLogin cancelled");
					}
					else if(strcmp(adminID, DEFAULT_ADMIN_ID) == 0 && strcmp(adminPassword, DEFAULT_ADMIN_PW) == 0)
					{
						UART_SendString("\r\nLogin Successful");
						break;
					}
					else
					{
						UART_SendString("\r\nWrong entry\r\n");
					}
				}
    	}

			UART_SendString("\r\nInitializing system for first time use...");
			E2prom_SystemSetup();
			UART_SendString("\r\nPlease change admin login info to continue initialization");
			do
			{
				UART_SendString("\r\nEnter Admin user name: ");
			}while(Terminal_ReceiveUserName(user_name) == CANCELLED);

			do
			{
				UART_SendString("\r\nEnter Admin ID: ");
			}while(Terminal_ReceiveUserID(user_id) == CANCELLED);

			do
			{
				UART_SendString("\r\nEnter Admin password: ");
			}while(Terminal_ReceiveUserPassword(user_password) == CANCELLED);

			if(E2prom_AddUser(user_id, user_name, user_password, admin) == FAIL)
			{
				UART_SendString("\r\nCould not initialize the system.. Please hard reset the system\r\n");
				while(1)
					{;}
			}
			else
			{
				UART_SendString("\r\nAdmin info updated successfully\r\nInitializing system..");
				E2prom_SetSystemFactorySetting(FACTORY_CONFIGURED);

			}

	}
	else
	{
		UART_SendString("Initializing system...");
		E2prom_SystemSetup();
	}

	UART_SendString("\r\nSystem initialized successfully\r\nEnter '@' to access the control menu");

	vTaskResume(keypadHandle);
	vTaskResume(lcdHandle);


	while(1)
	{
			xQueueReceive(Terminal_Queue, &terminalInput, portMAX_DELAY);
			switch(Terminal_CurrentState)
			{
				case Idle:
					if(terminalInput == '@')
					{
						Terminal_CurrentState = ReceivingAdminPassword;
						UART_SendString("\r\nEnter Admin/Moderator login info\r\n");
						xQueueSend(Terminal_Queue, 0, portMAX_DELAY);
					}
					else
						{;}
				break;

				case ReceivingAdminPassword:
					UART_SendString("\r\nEnter ID: ");

					if(Terminal_ReceiveUserID(adminID) == CANCELLED)
					{
						Terminal_CurrentState = Idle;
						UART_SendString("\r\nLogin cancelled\r\n");
						break; //break from case: Receiving admin password
					}
					else
						{;}

					UART_SendString("\r\nEnter password: ");

					if(Terminal_ReceiveUserPassword(adminPassword) == CANCELLED)
					{
						Terminal_CurrentState = Idle;
						UART_SendString("\r\nLogin cancelled\r\n");
						break; //break from case: Receiving admin password
					}
					else
						{;}


					loggedInRank = E2prom_VerifyAdminInfo(adminID, adminPassword);
					Terminal_CurrentState = MainMenu;
					switch(loggedInRank)
					{
						case admin:
							UART_SendString("\r\nLogged in as Administrator\r\n");
							break;
						case moderator:
							UART_SendString("\r\nLogged in as Moderator\r\n");
							break;
						default:
							Terminal_CurrentState = Idle;
							UART_SendString("\r\nWrong entry\r\n");
							break;
					}
					if(Terminal_CurrentState == MainMenu)
					{
						//UART_SendString("\r\n1.Unlock system\r\n2.Open door\r\n3.Close door\r\n4.Add/Remove user\r\n5.Exit\r\n");
					}
					else
						{;}
				break;

				case MainMenu:
					switch(terminalInput)
					{
						case '1':
							UART_SendString("\r\nSystem unlocked\r\n");
							//System unlock function

							break;
						case '2':
							UART_SendString("\r\nDoor opened\r\n");
							//open door function

							break;
						case '3':
							UART_SendString("\r\nDoor closed\r\n");
							//close door function

							break;
						case '4':
							if(loggedInRank == admin)
							{
								Terminal_CurrentState = UsersMenu;
								UART_SendString("\r\n1.Add User\r\n2.Modify User\r\n3.Remove User\r\n4.List Users\r\n5.Return to the previous menu\r\n");
							}
							else
							{
								UART_SendString("\r\nEnter Admin ID: ");
								if(Terminal_ReceiveUserID(adminID) == CANCELLED)
								{
									UART_SendString("\r\nLogin cancelled\r\n");
									break; //break from case '4'
								}
								else
									{;}

								UART_SendString("Enter Admin password: ");
								if(Terminal_ReceiveUserPassword(adminPassword) == CANCELLED)
								{
									UART_SendString("\r\nLogin cancelled\r\n");
									break; //break from case '4'
								}
								else
									{;}

								loggedInRank = E2prom_VerifyAdminInfo(adminID, adminPassword);
								if(loggedInRank == admin)
								{
									Terminal_CurrentState = UsersMenu;
										UART_SendString("\r\n1.Add User\r\n2.Modify User\r\n3.Remove User\r\n4.List Users\r\n4.Return to the previous menu\r\n");
								}
								else
								{
									UART_SendString("\r\nWrong Entry\r\n");
								}
								loggedInRank = moderator;
							}
							break;

						case '5':
							UART_SendString("\r\nLogged out\r\n");
							Terminal_CurrentState = Idle;
							break;

						default:
								UART_SendString("\r\n1.Unlock system\r\n2.Open door\r\n3.Close door\r\n4.Add/Remove user\r\n5.Exit\r\n");
						break;
					}

					break;

				case UsersMenu:
					switch(terminalInput)
					{
						case '1': //add new user
							UART_SendString("\r\nAdd new user\r\nEnter user name: ");

							if(Terminal_ReceiveUserName(user_name) == CANCELLED)
							{
								UART_SendString("\r\nAdding new user cancelled\r\n");
								break; //break from case '1' add new user
							}
							else
							{
								UART_SendString("\r\nEnter user ID: ");
							}

							if(Terminal_ReceiveUserID(user_id) == CANCELLED)
							{
								UART_SendString("\r\nAdding new user cancelled\r\n");
								break; //break from case '1' add new user
							}
							else
							{
								UART_SendString("\r\nEnter user password: ");
							}

							if(Terminal_ReceiveUserPassword(user_password) == CANCELLED)
							{
								UART_SendString("\r\nAdding new user cancelled\r\n");
								break; //break from case '1' add new user
							}
							else
								{;}

							UART_SendString("\r\nEnter user rank:\r\n1.Admin\r\n2.Moderator\r\n3.Member\r\nEsc. Cancel\r\n");

							if(Terminal_ReceiveUserRank(&user_rank) == CANCELLED)
							{
								UART_SendString("\r\nAdding new user cancelled\r\n");
								break; //break from case '1' add new user
							}
							else
								{;}

							if(E2prom_AddUser(user_id, user_name, user_password, user_rank) == SUCCESS)
							{
								UART_SendString("\r\nUser added successfully\r\n");
							}
							else
							{
								UART_SendString("\r\nCould not add user\r\n");
							}
						break;

						case '2': // modify user
							UART_SendString("\r\nModify user\r\nEnter user ID: ");
							i=0;
							while(1)
							{
								xQueueReceive(Terminal_Queue, &terminalInput, portMAX_DELAY);
								switch(terminalInput)
								{
									case '\r':
										if(i == 4)
										{
											user_id[i] = '\0';
											UART_SendString("\r\n");
										}
										else
											{;}
									break;

									case '\b':
										if(i>0)
										{
											i--;
											UART_SendString("\b \b");
										}
										else
											{;}
									break;

									default:
										if(i<4 || (terminalInput >= '0' && terminalInput <= '9'))
										{
											user_id[i] = terminalInput;
											i++;
											UART_SendChar(terminalInput);
										}
									break;
								}

								if(terminalInput == 0x1B || (terminalInput == '\r' && i == 4))
								{
									break; // break from while 1
								}
								else
									{;}

							}

							if(terminalInput == 0x1B)
							{
								UART_SendString("\r\nModifying user cancelled\r\n");
								break; //break from switch case to modify user
							}

							UART_SendString("\r\bEnter new user name (leave it blank to skip): ");
							i=0;
							while(1)
							{
								xQueueReceive(Terminal_Queue, &terminalInput, portMAX_DELAY);
								switch(terminalInput)
								{
									case '\r':
										if(i > 3)
										{
											user_name[i] = '\0';
											UART_SendString("\r\n");
										}
										else if(i == 0)
										{
											user_name[i] = '\0';
											UART_SendString("N/A\r\n");
										}
										else
											{;}
									break;

									case '\b':
										if(i>0)
										{
											i--;
											UART_SendString("\b \b");
										}
										else
											{;}
									break;

									default:
										if(i<16 && ((terminalInput >= 'A' && terminalInput <= 'Z') || (terminalInput >= 'a' && terminalInput <= 'z') || terminalInput == ' '))
										{
											user_name[i] = terminalInput;
											i++;
											UART_SendChar(terminalInput);
										}
										else
											{;}
									break;
								}

								if(terminalInput == 0x1B || ((terminalInput == '\r') && (i == 0 || i > 3)))
								{
									break; //break from while(1)
								}
								else
									{;}
							}

							if(terminalInput == 0x1B)
							{
								UART_SendString("\r\nModifying user cancelled\r\n");
								break; //break from switch case modifying user
							}
							else
								{;}

							UART_SendString("\r\bEnter new user password (leave it blank to skip): ");
							i=0;
							while(1)
							{
								xQueueReceive(Terminal_Queue, &terminalInput, portMAX_DELAY);
								switch(terminalInput)
								{
									case '\r':
										if(i == 4)
										{
											user_password[i] = '\0';
											UART_SendString("\r\n");
										}
										else if(i == 0)
										{
											user_password[i] = '\0';
											UART_SendString("N/A\r\n");
										}
										else
											{;}
									break;

									case '\b':
										if(i>0)
										{
											i--;
											UART_SendString("\b \b");
										}
										else
											{;}
									break;

									default:
										if(i<4 && ((terminalInput >= '0' && terminalInput <= '9')))
										{
											user_password[i] = terminalInput;
											i++;
											UART_SendChar(terminalInput);
										}
										else
											{;}
									break;
								}

								if(terminalInput == 0x1B || ((terminalInput == '\r') && (i == 4)))
								{
									break; //break from while(1)
								}
								else
									{;}
							}

							if(terminalInput == 0x1B)
							{
								UART_SendString("\r\nModifying user cancelled\r\n");
								break; //break from switch case modifying user
							}
							else
								{;}

							if(E2prom_ModifyUser(user_id, user_name, user_password) == FAIL)
							{
								UART_SendString("\r\nCould not modify user info\r\n");
							}
							else
							{
								UART_SendString("\r\nUser info modified successfully\r\n");
							}

						break;
						case '3': // remove user
							UART_SendString("Enter user ID to be removed: ");
							i=0;
							while(1)
							{
								xQueueReceive(Terminal_Queue, &terminalInput, portMAX_DELAY);
								switch(terminalInput)
								{
									case '\r':
										if(i==4)
										{
											user_id[i] = '\0';
											UART_SendString("\r\n");
										}
										else
											{;}
									break;

									case '\b':
										if(i>0)
										{
											i--;
											UART_SendString("\b \b");
										}
										else
											{;}
									break;

									default:
										if(i<4 && (terminalInput >= '0' && terminalInput <= '9'))
										{
											user_id[i] = terminalInput;
											i++;
											UART_SendChar(terminalInput);
										}
									break;
								}

								if(terminalInput == 0x1B || (i == 4 && terminalInput == '\r'))
								{
									break;
								}
								else
									{;}
							}

							if(terminalInput == 0x1B)
							{
								UART_SendString("\r\nRemoving user cancelled\r\n");
								break; // break from switch case removing user
							}
							else
								{;}

							if(E2prom_RemoveUser(user_id, adminID) == FAIL)
							{
								UART_SendString("\r\nCould not remove user\r\n");
							}
							else
							{
								UART_SendString("\r\nUser removed\r\n");
							}

						break;

						case '4': //list users
							E2prom_ListUsers();
						break;

						case '5': // return to the previous menu
							Terminal_CurrentState = MainMenu;
							UART_SendString("\r\nReturning to the previous menu\r\n");
							UART_SendString("\r\n1.Unlock system\r\n2.Open door\r\n3.Close door\r\n4.Add/Remove user\r\n5.Exit\r\n");
						break;

						default:
							UART_SendString("\r\n1.Add User\r\n2.Modify User\r\n3.Remove User\r\n4.List Users\r\n5.Return to the previous menu\r\n");
						break;
					}
				break;

				default:
				break;
			}
	}
}




void UART_ReceiveComplete(void)
{
	uint8_t data = UART0->DR;
	xQueueSend(Terminal_Queue, (void *)&data, portMAX_DELAY);
	UART0->ICR &=~(1U<<4);
}
