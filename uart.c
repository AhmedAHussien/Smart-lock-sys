#include "uart.h"
#include "TM4C123GH6PM.h"
#include <stdint.h>
#include <stdio.h>




void UART_Init(void)
{
    SYSCTL->RCGCUART |= (1<<R0); 
  SYSCTL->RCGCGPIO |= (1<<R0);
  
  UART0->CTL    &=~ (1<<UARTEN);
  UART0->IBRD   = 8;
  UART0->FBRD   = 44;
  UART0->LCRH   = (0x3 << 5);//  | (1<< 4); //enable FIFO
    
  GPIOA->AFSEL  |= (0x3 << 0);
  GPIOA->DEN    |= (0x3 << 0);
  
  GPIOA->PCTL |= 0x00000011;
  GPIOA->AMSEL &=~ (0x3 << 0);
  UART0->IM |= (1U<<4);
  NVIC->ISER[0] = (1U<<5);
  
  UART0->CTL    |= (1<<UARTEN) | (1U<<RXE) | (1U<<TXE);

  
}

void UART_SendChar(char data)
{
  while(!(UART0->FR & (1<<7)))
    ;
  UART0->DR = data;
}

void UART_SendString(char * string)
{
  unsigned char i = 0;
  while(string[i] != '\0')
  {
    UART_SendChar(string[i]);
    i++;
  }     
}

uint8_t UART_ReceiveChar(void)
{
  while(UART0->FR & (1U<<4));
  return (UART0->DR);
}