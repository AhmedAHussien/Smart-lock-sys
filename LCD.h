/*
 * LCD.h
 *
 * Created: 4/16/2017 9:58:25 PM
 *  Author: moibrahim
 */ 


#ifndef _LCD_H_
#define _LCD_H_


#define LCD_CLEAR_DISPLAY       0x01
#define LCD_RETURN_HOME         0x02
#define LCD_DISPLAY_ON          0x0C
#define LCD_DISPLAY_OFF         0x08
#define LCD_CURSOR_SHIFT_LEFT   0x10
#define LCD_CURSOR_SHIFT_RIGHT  0x14

#include <stdint.h>


extern void LCD_WriteChar(uint8_t character);
extern void LCD_WriteNewLine(uint8_t *string);
extern void LCD_WriteString(uint8_t *string);
extern void LCD_GoTo(uint8_t x, uint8_t y);
extern void LCD_Init(void);
extern void LCD_ReturnHome(void);
extern void LCD_ClearScreen(void);
extern void LCD_WriteCommand(uint8_t command);



#endif /* LCD_H_ */