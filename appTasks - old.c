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

static uint8_t g_Password[5] = {0};
static uint8_t g_ID[5] = {0};
uint8_t passwordIncorrectCounter = 0;
static const uint8_t g_CurrentPassword[5] = "1212";

xQueueHandle LCD_Queue = NULL;
xQueueHandle Terminal_Queue = NULL;
xSemaphoreHandle UartSemaphore = NULL;
xSemaphoreHandle CheckPasswordSemaphore = NULL;
extern TaskHandle_t keypadHandle;
extern TaskHandle_t lcdHandle;



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
            //xSemaphoreGive(CheckPasswordSemaphore);       //Unblock check password take
            
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
            g_Password[passwordIndex] = '\0';
            passwordIndex = 0;
            LCD_CurrentState = Submit;
            xSemaphoreGive(CheckPasswordSemaphore);       //Unblock check password take
            
            break;
            
          default:
            if(passwordIndex<4 && KeypadKey >= 0x30 && KeypadKey <= 0x39)
            {
              g_Password[passwordIndex] = KeypadKey;
              passwordIndex++;
              LCD_WriteChar(KeypadKey);
              queueState = xQueuePeek(LCD_Queue, &KeypadKey, 1000/portTICK_PERIOD_MS);
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


void vTask_PasswordCheck(void * args)
{
    
	uint8_t* user_name = 0;
	xSemaphoreTake(CheckPasswordSemaphore, 0);
	while(1)
	{
		xSemaphoreTake(CheckPasswordSemaphore, portMAX_DELAY);
        vTaskSuspend(keypadHandle);
        vTaskSuspend(lcdHandle);
        
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
                vTaskResume(lcdHandle);
                vTaskResume(keypadHandle);
                
                
	}
}

void vTask_Terminal(void * args)
{
  
}



void UART_ReceiveComplete(void)
{
	uint8_t data = UART0->DR;
    xQueueSend(Terminal_Queue, (void *)&data, portMAX_DELAY);


  UART0->ICR &=~(1U<<4);
}