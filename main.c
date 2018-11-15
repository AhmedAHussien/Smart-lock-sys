#include "FreeRTOS.h"
#include "TM4C123GH6PM.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "userdata.h"
#include "uart.h"
#include <stdint.h>
#include "i2c.h"
#include "appTasks.h"



#ifdef DEBUG
static void UpdateArray(uint8_t * ptr);

static void UpdateArray(uint8_t * ptr)
{
  uint16_t i = 0;
  I2C_StartReceive(0x000);
  for(i=0; i<300; i++)
  {
    ptr[i] = I2C_ReceiveAck();
  }
  I2C_StopReceive();
}
#endif

extern xQueueHandle LCD_Queue;
extern xQueueHandle Terminal_Queue;
extern xSemaphoreHandle UartSemaphore;
TaskHandle_t keypadHandle = NULL;
TaskHandle_t lcdHandle = NULL;
TaskHandle_t terminalHandle = NULL;

int main()
{


   unsigned long int i = 0;
  for(i=0; i<20000000; i++);            //delay

  #ifdef DEBUG_GPIO
  SYSCTL->RCGCGPIO |= (1U<<5);
  GPIOF->DEN |= (1U<<4) | (1U<<3) | (1U<<2);
  GPIOF->DIR |= (1U<<4) | (1U<<3) | (1U<<2);
  GPIOF->DATA_BITS[1U<<4 | (1U<<3) | (1U<<2)] = (1U<<4) | (1U<<3) | (1U<<2);

  SYSCTL->RCGCGPIO |= (1U<<0);  //portA pin 0 [Buzzer]
  GPIOA->DEN |= (1U<<5);
  GPIOA->DIR |= (1U<<5);

  SYSCTL->RCGCGPIO |= (1U<<5);
  GPIOF->DEN |= (1U<<4);
  GPIOF->DIR |= (1U<<4);
  GPIOF->DATA_BITS[1U<<4] = (1U<<4);
  #endif


  I2C_Init();                    //initialize i2c
  UART_Init();

  SYSCTL->RCGCGPIO |= (1U<<1);
  GPIOB->DEN |= (1U<<1) | (1U<<0);
  GPIOB->DIR |= (1U<<1) | (1U<<0);

  GPIOB->DATA_BITS[(1U<<1) | (1U<<0)] = (1U<<1) | (1U<<0);

#ifdef DEBUG
  if(E2prom_SystemSetup() == FAIL)
  {
    while(1);
  }


  uint8_t arr[2001] = {0};

  UpdateArray(arr);
  arr[2000]++;

  E2prom_AddUser("1210", "Ahmed hessein", "1276", admin);
  UpdateArray(arr);
  arr[2000]++;
  E2prom_AddUser("1210", "Ahmed Nabil", "5060", moderator);
  UpdateArray(arr);
  arr[2000]++;
  E2prom_AddUser("1510", "Ahmed Fathy", "7070", admin);
  UpdateArray(arr);
  arr[2000]++;
  E2prom_AddUser("1520", "Heshmat", "7070", admin);
  UpdateArray(arr);
  arr[2000]++;
  E2prom_AddUser("5030", "Omar", "4561", moderator);
  UpdateArray(arr);
  arr[2000]++;
  E2prom_AddUser("7899", "Joe", "1000", member);
  UpdateArray(arr);
  arr[2000]++;
  E2prom_RemoveUser("1510", "1510");
  UpdateArray(arr);
  arr[2000]++;
  E2prom_RemoveUser("1510", "1210");
  UpdateArray(arr);
  arr[2000]++;
  E2prom_RemoveUser("1210", "1210");
  UpdateArray(arr);

  E2prom_ModifyUser("7899", "Yousef", "2000");
  UpdateArray(arr);
  arr[2000]++;
  E2prom_ModifyUser("5030", "Abdelrhman", "5060");
  UpdateArray(arr);
  arr[2000]++;

#endif
  //system start up function


  LCD_Queue = xQueueCreate(2, sizeof(uint8_t));
  Terminal_Queue = xQueueCreate(5, sizeof(uint8_t));
  UartSemaphore = xSemaphoreCreateBinary();

  xTaskCreate(vTask_LCD, "LCD", 250, NULL, 3, &lcdHandle);
  xTaskCreate(vTask_Keypad, "Keypad", 250, NULL, 2, &keypadHandle);
  xTaskCreate(vTask_Terminal, "Terminal", 250, NULL, 3, &terminalHandle);

  #ifdef DEBUG_GPIO
  GPIOA->DATA_BITS[(1U<<5)] = (1U<<5);
  #endif

  vTaskStartScheduler();

  for(;;);

  return 0;
}

















/*
void vTaskSwitch(void * args)
{
  uint8_t switchFlag = 0;
  TickType_t xLastWakeTime;
  xLastWakeTime = xTaskGetTickCount();
  portBASE_TYPE sendState;
  uint8_t color = 1;
  for(;;)
  {
    if((!((GPIOF->DATA_BITS[SW2]) & SW2)) && switchFlag == 0){
      switchFlag = 1;
      sendState = xQueueSendToBack(myQueue, (void *)&color, 0xFFFFFFFF);
      if(sendState == pdPASS){
        color += 1;
        if(color >= 8)
          color = 1;       else ;                  //Do Nothing
      }
      else
        UART_SendString("Could not send to the queue\r\n");
    }  else ;

    if(GPIOF->DATA_BITS[SW2] & SW2)
      switchFlag = 0;    else ;

    vTaskDelayUntil(&xLastWakeTime, 50/portTICK_PERIOD_MS);
  }
}


void vTaskLED(void * args)
{
  portBASE_TYPE receivedState;
  uint8_t  receivedColor = 0x00;
  TickType_t xLastWakeTime;
  xLastWakeTime = xTaskGetTickCount();
  for(;;)
  {
    receivedState = xQueueReceive(myQueue, &receivedColor, 0);
    if(receivedState == pdPASS)
      GPIOF->DATA_BITS[(0x07 << 1U)] &=~ (0x07 << 1U);  else ;

      GPIOF->DATA_BITS[(0x07 << 1U)] |= (receivedColor << 1U);
      vTaskDelayUntil(&xLastWakeTime, (500/portTICK_PERIOD_MS));
      GPIOF->DATA_BITS[(0x07 << 1U)] &=~ (receivedColor << 1U);
      vTaskDelayUntil(&xLastWakeTime, (500/portTICK_PERIOD_MS));
  }
}
*/
