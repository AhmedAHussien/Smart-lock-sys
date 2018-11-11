#include "uart.h"
#include <stdio.h>
#include "TM4C123GH6PM.h"

volatile  char data = 0;

int main()
{
  UART_Init();
  NVIC->ISER[0] = (1U<<5);
  __enable_interrupt();
  
  UART_SendChar('7');
      UART_SendString("Hello world, hey");

  while(1)
  {
  }
  
  return 0;
}

