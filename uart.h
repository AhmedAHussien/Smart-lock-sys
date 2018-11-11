#ifndef _UART_H_
#define _UART_H_

#include <stdint.h>

#define UARTEN  0
#define RXE     9
#define TXE     8
void UART_Init(void);
void UART_SendChar(char data);
void UART_SendString(char * string);
uint8_t UART_ReceiveChar(void);
void UART_ReceiveComplete(void);

#endif