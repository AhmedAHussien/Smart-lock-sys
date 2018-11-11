/*
 * keypad.h
 *
 * Created: 3/29/2018 8:24:24 PM
 *  Author: Peter
 */ 


#ifndef KEYPAD_H_
#define KEYPAD_H_

#include "utils.h"
void Keypad_Init (void);
unsigned char Keypad_ReadChar (void);

#define NO_OUTPUT_ROW 1
#define OUTPUT_ROW 0
#define DEFAULT_STATE_COL 1
#define INPUT_COL 0
#define NOTPRESSED 0xFF
#define CHECK_COL_INPUT !(GET_BIT(COL0_PORT->DATA,COL0) && GET_BIT(COL1_PORT->DATA,COL1) && GET_BIT(COL2_PORT->DATA,COL2) && GET_BIT(COL3_PORT->DATA,COL3))
						
						
#endif /* KEYPAD_H_ */