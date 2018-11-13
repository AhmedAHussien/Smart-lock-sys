#include "freeRTOS.h"
#include "task.h"
#include "queue.h"
#include "uart.h"
#include "terminal.h"

extern xQueueHandle Terminal_Queue;


extern uint8_t Terminal_ReceiveUserID(uint8_t * user_id)
{
	uint8_t terminalInput = 0;
	uint8_t i = 0;
	while(1)
	{
		xQueueReceive(Terminal_Queue, &terminalInput, portMAX_DELAY);
		switch(terminalInput)
		{
			case '\r':
				if(i == 4)
				{
					user_id[i] = '\0';
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
				if(terminalInput >= 0x30 && terminalInput <= 0x39 && i < 4)
				{
					user_id[i] = terminalInput;
					i++;
					UART_SendChar(terminalInput);
				}
				else
					{;}
			break;
		}

		if(terminalInput == 0x1B)
		{
			return CANCELLED;
		}
		else if(i == 4 & terminalInput == '\r')
		{
			return SUCCESS;
		}
		else
			{;}
	}

}

extern uint8_t Terminal_ReceiveUserPassword(uint8_t * user_password)
{
	uint8_t terminalInput = 0;
	uint8_t i = 0;
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
				if(terminalInput >= 0x30 && terminalInput <= 0x39 && i < 4)
				{
					user_password[i] = terminalInput;
					i++;
					UART_SendChar(terminalInput);
					xQueuePeek(Terminal_Queue, &terminalInput, 1000/portTICK_PERIOD_MS);
					UART_SendString("\b*");
				}
				else
					{;}
			break;
		}

		if(terminalInput == 0x1B)
		{
			return CANCELLED;
		}
		else if(i == 4 & terminalInput == '\r')
		{
			return SUCCESS;
		}
		else
			{;}
	}

}

extern uint8_t Terminal_ReceiveUserName(uint8_t * user_name)
{
	uint8_t terminalInput = 0;
	uint8_t i = 0;
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

		if(terminalInput == 0x1B)
		{
			return CANCELLED;
		}
		else if(i > 3 & terminalInput == '\r')
		{
			return SUCCESS;
		}
		else
			{;}
	}

}

extern uint8_t Terminal_ReceiveUserRank(Rank * user_rank)
{
	uint8_t terminalInput = 0;
	do
	{
		xQueueReceive(Terminal_Queue, &terminalInput, portMAX_DELAY);
	}while(!((terminalInput >= '1' && terminalInput <= '3') || terminalInput == 0x1B));

	switch(terminalInput)
	{
		case '1':
			*user_rank = admin;
			break;

		case '2':
			*user_rank = moderator;
			break;

		case '3':
			*user_rank = member;
			break;

		default:
			return CANCELLED;
			break;
	}

	return SUCCESS;
}
