#include "FreeRTOS.h"
#include "TM4C123GH6PM.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include <stdint.h>
#include "appTasks.h"




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



  LCD_Queue = xQueueCreate(2, sizeof(uint8_t));
  Terminal_Queue = xQueueCreate(5, sizeof(uint8_t));
  UartSemaphore = xSemaphoreCreateBinary();

  xTaskCreate(vTask_LCD, "LCD", 250, NULL, 3, &lcdHandle);
  xTaskCreate(vTask_Keypad, "Keypad", 250, NULL, 2, &keypadHandle);
  xTaskCreate(vTask_Terminal, "Terminal", 250, NULL, 3, &terminalHandle);



  vTaskStartScheduler();

  for(;;);

  return 0;
}
