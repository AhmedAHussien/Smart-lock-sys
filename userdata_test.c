#include "i2c.h"
#include "userdata.h"
#include <stdint.h>
#include <stdio.h>

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

int main()
{
  unsigned long int i = 0;
  for(i=0; i<20000000; i++);            //delay
  
  I2C_Init();                    //initialize i2c

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
    printf("Hello\nHey");
  

  while(1);
  
  return 0;
}