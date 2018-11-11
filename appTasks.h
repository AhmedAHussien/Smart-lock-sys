#ifndef _APPTASKS_H_
#define _APPTASKS_H_


typedef enum
{
  Welcome,
  Receiving_ID,
  Receiving_Password,
  Submit
}LCD_State_t;

typedef enum
{
	Idle,
	ReceivingAdminPassword,
	MainMenu,
	UsersMenu
}Terminal_State_t;


void vTask_Keypad(void * args);
void vTask_LCD(void * args);
void vTask_PasswordCheck(void * args);
void vTask_Terminal(void * args);

#endif