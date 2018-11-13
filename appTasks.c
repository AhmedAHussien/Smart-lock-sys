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
		uint8_t user_id[5];
		uint8_t user_password[5];
		uint8_t user_name[17];
		uint8_t i = 0;
		BaseType_t queueState = pdFALSE;
		LCD_State_t LCD_CurrentState = Welcome;

		LCD_Init();
		LCD_WriteNewLine("Welcome");

		xSemaphoreGive(UartSemaphore);

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
						LCD_CurrentState = Receiving_ID;
						i = 0;
						LCD_WriteNewLine("Enter ID:");
						LCD_GoTo(0,1);
					}
					break;

					case Receiving_ID:
					switch(KeypadKey)
					{
						case 'C':
						if(i > 0)
						{
							i--;
							LCD_WriteCommand(LCD_CURSOR_SHIFT_LEFT);
							LCD_WriteChar(' ');
							LCD_WriteCommand(LCD_CURSOR_SHIFT_LEFT);
							//user_id[i] = '\0';
						}
						break;

						case '#':
						if(i == 4)
						{
							user_id[i] = '\0';
							i = 0;
							LCD_CurrentState = Receiving_Password;
							LCD_WriteNewLine("Enter password:");
							LCD_GoTo(0,1);
						}
						break;

						default:
						if(i < 4 && KeypadKey >= 0x30 && KeypadKey <= 0x39)
						{
							user_id[i] = KeypadKey;
							i++;
							LCD_WriteChar(KeypadKey);
						}
						break;
					}
					break;

					case Receiving_Password:
					switch(KeypadKey)
					{
						case 'C':
						if(i > 0)
						{
							i--;
							LCD_WriteCommand(LCD_CURSOR_SHIFT_LEFT);
							LCD_WriteChar(' ');
							LCD_WriteCommand(LCD_CURSOR_SHIFT_LEFT);
							//user_password[i] = '\0';
						}
						else
						{;}

						break;

						case '#':
						if(i == 4)
						{
							vTaskSuspend(keypadHandle);
							user_password[i] = '\0';
							i = 0;
							LCD_CurrentState = Submit;

							if(uxSemaphoreGetCount(UartSemaphore) == 0)
							{
								LCD_WriteNewLine("Please Standby");
							}
							else
							{;}

							if(xSemaphoreTake(UartSemaphore, 12000/portTICK_PERIOD_MS) == pdFALSE)
							{
								//Semaphore is not taken in this block of code and will exist by the break statement
								LCD_WriteNewLine("Error: System");
								LCD_GoTo(0,1);
								LCD_WriteString("maintenance");
								vTaskResume(keypadHandle);
								//vTaskDelay(3000/portTICK_PERIOD_MS);
								break;
							}
							else
							{
								//Semaphore is taken from this point
								if(E2prom_VerifyUserInfo(user_id, user_password) == VALID)
								{
									E2prom_ResetIncorrectPWCounter();

									E2prom_GetUserName(user_id, user_name);
									UART_SendString("\r\n");
									UART_SendString(user_name);
									UART_SendString(" has logged in\r\n");
									xSemaphoreGive(UartSemaphore);

									LCD_WriteNewLine("Welcome");
									LCD_GoTo(0,1);
									LCD_WriteString(user_name);

									//Green LED on & open door
									//wait 2 seconds
									//stop door
									//wait 2 seconds
									//close door
									//wait 2 seconds
									//Green LED off & stop door

									vTaskDelay(6000/portTICK_PERIOD_MS);
								}
								else
								{
									E2prom_IncrementIncorrectPWCounter();			//semaphore is taken
									UART_SendString("Wrong entry with ID: ");
									UART_SendString(user_id);
									UART_SendString(" - PW: ");
									UART_SendString(user_password);
									UART_SendString("\n\r");
									if(E2prom_GetIncorrectPWCounter() >= 3)
									{
										E2prom_SetSystemState(LOCKED);
										UART_SendString("3 Wrong Entries, System lock down\n\r");
									}
									else
									{;}

									if(E2prom_GetSystemState() == LOCKED)
									{
										LCD_WriteNewLine("Access Denied");
										while(E2prom_GetSystemState() == LOCKED)
										{
											xSemaphoreGive(UartSemaphore);
											vTaskDelay(500/portTICK_PERIOD_MS);
											LCD_WriteCommand(LCD_DISPLAY_OFF);
											vTaskDelay(500/portTICK_PERIOD_MS);
											LCD_WriteCommand(LCD_DISPLAY_ON);
											xSemaphoreTake(UartSemaphore, portMAX_DELAY);
										}
										xSemaphoreGive(UartSemaphore);
									}
									else
									{
										LCD_WriteNewLine("Wrong Entires: ");
										LCD_WriteChar(E2prom_GetIncorrectPWCounter() + 0x30);
										xSemaphoreGive(UartSemaphore);
										vTaskDelay(3000/portTICK_PERIOD_MS);
									}

								}

							}
							vTaskResume(keypadHandle);

						}
						else
						{;}
						break;

						default:
						if(i<4 && KeypadKey >= 0x30 && KeypadKey <= 0x39)
						{
							user_password[i] = KeypadKey;
							i++;
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
					i = 0;
					LCD_WriteNewLine("Welcome");
					break;
				}

			}
			else
			{
				if(LCD_CurrentState != Welcome)
				{
					LCD_CurrentState = Welcome;
					i = 0;
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
		uint8_t adminUserName[17];

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
					UART_SendString("\r\nEnter Admin/Moderator login info");
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

				if(E2prom_VerifyUserInfo(adminID, adminPassword) == VALID)
				{
					E2prom_GetUserRank(adminID, &loggedInRank);
					switch(loggedInRank)
					{
						case admin:
						E2prom_GetUserName(adminID, adminUserName);
						UART_SendString("\r\nLogged in as Administrator. Welcome ");
						UART_SendString(adminUserName);
						Terminal_CurrentState = MainMenu;
						break;

						case moderator:
						E2prom_GetUserName(adminID, adminUserName);
						UART_SendString("\r\nLogged in as Moderator. Welcome ");
						UART_SendString(adminUserName);
						Terminal_CurrentState = MainMenu;
						break;

						case member:
						Terminal_CurrentState = Idle;
						UART_SendString("\r\nUnauthorized\r\n");
						break;
					}
				}
				else
				{
					Terminal_CurrentState = Idle;
					UART_SendString("\r\nWrong entry\r\n");
				}

				break;

				case MainMenu:
				switch(terminalInput)
				{
					case '1':
					E2prom_ResetIncorrectPWCounter();
					E2prom_SetSystemState(UNLOCKED);
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

						UART_SendString("\r\nEnter Admin password: ");
						if(Terminal_ReceiveUserPassword(adminPassword) == CANCELLED)
						{
							UART_SendString("\r\nLogin cancelled\r\n");
							break; //break from case '4'
						}
						else
						{;}

						E2prom_GetUserRank(adminID, &loggedInRank);
						if(E2prom_VerifyUserInfo(adminID, adminPassword) == VALID)
						{
							E2prom_GetUserRank(adminID, &loggedInRank);
							if(loggedInRank == admin)
							{
								UART_SendString("\r\nUsers menu access granted\r\n");
								Terminal_CurrentState = UsersMenu;
							}
							else
							{
								UART_SendString("\r\nUnauthorized\r\n");
							}
							//UART_SendString("\r\n1.Add User\r\n2.Modify User\r\n3.Remove User\r\n4.List Users\r\n4.Return to the previous menu\r\n");
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

					if(Terminal_ReceiveUserID(user_id) == CANCELLED)
					{
						UART_SendString("\r\nModifying user cancelled\r\n");
						break; //break from case '2' modify user
					}
					else
					{;}

					UART_SendString("\r\nEnter new user name (leave it blank to skip): ");
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
							}
							else if(i == 0)
							{
								user_name[i] = '\0';
								UART_SendString("N/A");
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

					UART_SendString("\r\nEnter new user password (leave it blank to skip): ");
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
							}
							else if(i == 0)
							{
								user_password[i] = '\0';
								UART_SendString("N/A");
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

						if(terminalInput == 0x1B || ((terminalInput == '\r') && (i == 0 || i == 4)))
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

					if(Terminal_ReceiveUserID(user_id) == CANCELLED)
					{
						UART_SendString("\r\nRemoving user cancelled\r\n");
						break; //break from case '3' remove user
					}
					else
					{;}

					if(strcmp(user_id, adminID) == 0)
					{
						UART_SendString("\r\nCannot remove currently logged in admin\r\n");
					}
					else if(E2prom_RemoveUser(user_id) == FAIL)
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
