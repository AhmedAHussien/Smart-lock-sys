/*
 * LCD.c
 *
 * Created: 4/16/2017 10:00:05 PM
 *  Author: moibrahim
 */ 

#include <stdint.h>             
#include "TM4C123GH6PM.h"         
#include "LCD.h"
#include "lcd_config.h"
#include "freeRTOS.h"
#include "task.h"

#define R0      0
#define R1      1
#define R2      2
#define R3      3
#define R4      4
#define R5      5

#define LINE1_END 15
#define LINE2_END 31



extern void LCD_Init(void)
{
    SYSCTL->RCGCGPIO |= (1U<<R0) | (1U<<R2);    //enable clock gating on PORT A,B
    LCD_DATA->DEN |= (0xF << 4U);          //PORTC LCD data line digital enable
    LCD_DATA->DIR |= (0xF << 4U);          //set PORTC LCD data line as output
    
    LCD_CTRL->DEN |= (0x7 << 2U);          //PORTA LCD control line digital enable
    LCD_CTRL->DIR |= (0x7 << 2U);          //set PORTA LCD control line as output
    LCD_CTRL->DATA_BITS[(1U<<EN)|(1U<<RW)|(1U<<RS)] = 0x00;
    LCD_DATA->DATA = 0x00;

	vTaskDelay(70/portTICK_PERIOD_MS);
	LCD_CTRL->DATA_BITS[(1U<<EN)|(1U<<RW)|(1U<<RS)] &=~ (1U<<RW);
	LCD_CTRL->DATA_BITS[(1U<<EN)|(1U<<RW)|(1U<<RS)] &=~ (1U<<RS);
	LCD_DATA->DATA = 0x30;
	LCD_CTRL->DATA_BITS[(1U<<EN)|(1U<<RW)|(1U<<RS)] |= (1U<<EN);
	vTaskDelay(6/portTICK_PERIOD_MS);
	LCD_CTRL->DATA_BITS[(1U<<EN)|(1U<<RW)|(1U<<RS)] &=~ (1U<<EN);

	LCD_DATA->DATA = 0x30;
	LCD_CTRL->DATA_BITS[(1U<<EN)|(1U<<RW)|(1U<<RS)] |= (1U<<EN);
	vTaskDelay(2/portTICK_PERIOD_MS);
	LCD_CTRL->DATA_BITS[(1U<<EN)|(1U<<RW)|(1U<<RS)] &=~ (1U<<EN);

	LCD_DATA->DATA = 0x30;
	LCD_CTRL->DATA_BITS[(1U<<EN)|(1U<<RW)|(1U<<RS)] |= (1U<<EN);
	vTaskDelay(2/portTICK_PERIOD_MS);
	LCD_CTRL->DATA_BITS[(1U<<EN)|(1U<<RW)|(1U<<RS)] &=~ (1U<<EN);

	LCD_DATA->DATA = 0x20;
	LCD_CTRL->DATA_BITS[(1U<<EN)|(1U<<RW)|(1U<<RS)] |= (1U<<EN);
	vTaskDelay(2/portTICK_PERIOD_MS);
	LCD_CTRL->DATA_BITS[(1U<<EN)|(1U<<RW)|(1U<<RS)] &=~ (1U<<EN);

	LCD_WriteCommand(0x28);
	LCD_WriteCommand(0x08);
	LCD_WriteCommand(0x01);
	LCD_WriteCommand(0x06);
	LCD_WriteCommand(0x0C);

}

extern void LCD_WriteChar(uint8_t character)
{
	LCD_CTRL->DATA_BITS[(1U<<EN)|(1U<<RW)|(1U<<RS)] |= (1U<<RS);		// RS=1 select data register to write in it
	LCD_CTRL->DATA_BITS[(1U<<EN)|(1U<<RW)|(1U<<RS)] &=~ (1U<<RW);		//RW=0
	LCD_DATA->DATA = character;		//DataBus = command
	LCD_CTRL->DATA_BITS[(1U<<EN)|(1U<<RW)|(1U<<RS)] |= (1U<<EN);		//E=1
	vTaskDelay(2/portTICK_PERIOD_MS);
	LCD_CTRL->DATA_BITS[(1U<<EN)|(1U<<RW)|(1U<<RS)] &=~ (1U<<EN);		//E=0

	LCD_DATA->DATA = (character << 4U);	//DataBus = command
	LCD_CTRL->DATA_BITS[(1U<<EN)|(1U<<RW)|(1U<<RS)] |= (1U<<EN);			//E=1
	vTaskDelay(2/portTICK_PERIOD_MS);
	LCD_CTRL->DATA_BITS[(1U<<EN)|(1U<<RW)|(1U<<RS)] &=~ (1U<<EN);			//E=0
}

extern void LCD_WriteNewLine(uint8_t *string)
{
	LCD_WriteCommand(LCD_CLEAR_DISPLAY);
	uint8_t localIterator = 0;
	for(localIterator = 0; (string[localIterator] !='\0') && (localIterator <= LINE2_END); localIterator++)
	{
		if(localIterator == (LINE2_END - LINE1_END))
		{
			LCD_GoTo(0,1);
		}
		LCD_WriteChar(string[localIterator]);
	}
}

extern void LCD_WriteString(uint8_t *string)
{
	uint8_t localIterator = 0;
	for(localIterator = 0; (string[localIterator] !='\0') && (localIterator <= LINE2_END); localIterator++)
	{
		if(localIterator == (LINE2_END - LINE1_END))
		{
			LCD_GoTo(0,1);
		}
		LCD_WriteChar(string[localIterator]);
	}
}


extern void LCD_WriteCommand(uint8_t command)
{
	LCD_CTRL->DATA_BITS[(1U<<EN)|(1U<<RW)|(1U<<RS)] &=~ (1U<<RS); // RS=0 select command register to write in it
	LCD_CTRL->DATA_BITS[(1U<<EN)|(1U<<RW)|(1U<<RS)] &=~ (1U<<RW); //RW=0
	LCD_DATA->DATA = command; //DataBus = command
	LCD_CTRL->DATA_BITS[(1U<<EN)|(1U<<RW)|(1U<<RS)] |= (1U<<EN);  //E=1
	vTaskDelay(2/portTICK_PERIOD_MS);
	LCD_CTRL->DATA_BITS[(1U<<EN)|(1U<<RW)|(1U<<RS)] &=~ (1U<<EN);  //E=0
        
	LCD_DATA->DATA = (command << 4U); //DataBus = command
	LCD_CTRL->DATA_BITS[(1U<<EN)|(1U<<RW)|(1U<<RS)] |= (1U<<EN);  //E=1
	vTaskDelay(2/portTICK_PERIOD_MS);
	LCD_CTRL->DATA_BITS[(1U<<EN)|(1U<<RW)|(1U<<RS)] &=~ (1U<<EN);  //E=0
}

extern void LCD_GoTo(uint8_t x, uint8_t y)
{
	
	if(0 == y)
	{
		if (x <=15)
		{
			LCD_WriteCommand(0x80 + x);
		}
	}
	else if (1 == y)
	{
		if (x <=15)
		{
			LCD_WriteCommand(0xC0 + x);
		}
	}
}



extern void LCD_ReturnHome(void)
{
	LCD_WriteCommand(0x02);
}