/*
 * keypad.c
 *
 * Created: 3/29/2018 8:24:11 PM
 *  Author: Peter
 */ 
#include "keypad_config.h"
#include "utils.h"
#include "keypad.h"
#include "TM4C123GH6PM.h"
/*
function name:
description :




*/
#define R0      0
#define R1      1
#define R2      2
#define R3      3
#define R4      4
#define R5      5

void Keypad_Init (void)
{
  SYSCTL->RCGCGPIO |= (1U<<R3) | (1U<<R4) | (1U<<R5);
  //digital enable
  ROW0_PORT->DEN |= (1U<<ROW0);
  ROW1_PORT->DEN |= (1U<<ROW1);
  ROW2_PORT->DEN |= (1U<<ROW2);
  ROW3_PORT->DEN |= (1U<<ROW3);
  COL0_PORT->DEN |= (1U<<COL0);
  COL1_PORT->DEN |= (1U<<COL1);
  COL2_PORT->DEN |= (1U<<COL2);
  COL3_PORT->DEN |= (1U<<COL3);
  
	// set row direction as output
  ROW0_PORT->DIR |= (1U<<ROW0);
  ROW1_PORT->DIR |= (1U<<ROW1);
  ROW2_PORT->DIR |= (1U<<ROW2);
  ROW3_PORT->DIR |= (1U<<ROW3);
	// set col direction as input
  COL0_PORT->DIR &=~ (1U<<COL0);
  COL1_PORT->DIR &=~ (1U<<COL1);
  COL2_PORT->DIR &=~ (1U<<COL2);
  COL3_PORT->DIR &=~ (1U<<COL3);
        // set the initial value (default) for input

  COL0_PORT->PUR |= (1U<<COL0);
  COL1_PORT->PUR |= (1U<<COL1);
  COL2_PORT->PUR |= (1U<<COL2);
  COL3_PORT->PUR |= (1U<<COL3);
	// set the no output state for row output
  ROW0_PORT->DATA_BITS[1U<<ROW0] |= (1U<<ROW0);
  ROW1_PORT->DATA_BITS[1U<<ROW1] |= (1U<<ROW1);
  ROW2_PORT->DATA_BITS[1U<<ROW2] |= (1U<<ROW2);
  ROW3_PORT->DATA_BITS[1U<<ROW3] |= (1U<<ROW3);
	
}

unsigned char Keypad_ReadChar (void)
{
	static unsigned char gKeyMap [4][4]={
          {'1','2','3','A'},	
          {'4','5','6','B'},
          {'7','8','9','C'},
          {'*','0','#','D'}};
        
	uint8_t row, col=NOTPRESSED, result=NOTPRESSED;
	//loop on row
	for (row=0;row<4;row++)
	{	
		
	// set the no output state for row output
          ROW0_PORT->DATA_BITS[1U<<ROW0] |= (1U<<ROW0);
          ROW1_PORT->DATA_BITS[1U<<ROW1] |= (1U<<ROW1);
          ROW2_PORT->DATA_BITS[1U<<ROW2] |= (1U<<ROW2);
          ROW3_PORT->DATA_BITS[1U<<ROW3] |= (1U<<ROW3);
	// set the output for selected pin
		
          switch (row)
          {
            case 0: ROW0_PORT->DATA_BITS[1U<<ROW0] &=~ (1U<<ROW0);  break;
            case 1: ROW1_PORT->DATA_BITS[1U<<ROW1] &=~ (1U<<ROW1);  break;
            case 2: ROW2_PORT->DATA_BITS[1U<<ROW2] &=~ (1U<<ROW2);  break;
            case 3: ROW3_PORT->DATA_BITS[1U<<ROW3] &=~ (1U<<ROW3);  break;
            default: 
              break;        /*MISRA*/		
            }
            //check if there valid input
		if (GET_BIT(COL0_PORT->DATA_BITS[(1U<<COL0)],COL0)==INPUT_COL)
                  col = 0;
		else if (GET_BIT(COL1_PORT->DATA_BITS[(1U<<COL1)],COL1)==INPUT_COL)
                  col = 1;
		else if (GET_BIT(COL2_PORT->DATA_BITS[(1U<<COL2)],COL2)==INPUT_COL)
                  col = 2;
		else if (GET_BIT(COL3_PORT->DATA_BITS[(1U<<COL3)],COL3)==INPUT_COL)
                  col = 3;
		else ;/*MISRA*/
                
            if (CHECK_COL_INPUT)
		result = gKeyMap [row][col];
			
		//end for
	}
	// return mapped key
	return result;
}