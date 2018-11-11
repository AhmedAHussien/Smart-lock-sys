#include "TM4C123GH6PM.h"
#include <stdint.h>
#include "i2c.h"

extern void I2C_Init(void)
{
  SYSCTL->RCGCI2C |= (1U<<R1);
  SYSCTL->RCGCGPIO |= (1U<<R0);
  GPIOA->DEN |= (1U<<7) | (1U<<6);
  GPIOA->AFSEL |= (1U<<7) | (1U<<6);
  GPIOA->ODR |= (1U<<7);
  GPIOA->PCTL |= 0x33000000;
  
  I2C1->MCR = (1U<<MFE);
  I2C1->MTPR = (0x1<<TPRate);
}

extern uint8_t I2C_StartTransmit(uint16_t eepromAddress)
{
  while(I2C1->MCS & (1U<<BUSBSY));
  I2C1->MSA = EEPROM_WRITE_ADDRESS;
  I2C1->MDR = (eepromAddress >> 8U);
  
  
  I2C1->MCS = (1U<<START) | (1U<<RUN);
  
  while(I2C1->MCS & (1U<<BUSY));
  if(I2C1->MCS & (1U<<ERROR))
    return errCOULD_NOT_GENERATE_START;
  
  I2C1->MDR = eepromAddress;
 
  I2C1->MCS = (1U<<RUN);
  while(I2C1->MCS & (1U<<BUSY));
  if(I2C1->MCS & (1U<<ERROR))
    return errCOULD_NOT_GENERATE_START;
  return SUCCESS;
}

extern uint8_t I2C_StartReceive(uint16_t eepromAddress)
{
  if(I2C_StartTransmit(eepromAddress) == errCOULD_NOT_GENERATE_START)
    return errCOULD_NOT_GENERATE_START;
  //repeated start
  I2C1->MSA = EEPROM_READ_ADDRESS;
  I2C1->MCS = (1U<<ACK) | (1U<< START) | (1U<<RUN);
  while(I2C1->MCS & (1U<<BUSY));
  if(I2C1->MCS & (1U<<ERROR))
    return errCOULD_NOT_GENERATE_START;
  return I2C1->MDR;
}

extern void I2C_Transmit(uint8_t data)
{
  
  while(I2C1->MCS & (1U<<BUSY));
  I2C1->MDR = data;
  I2C1->MCS = (1U<<RUN);
}


extern uint8_t I2C_ReceiveAck(void)
{
  while(I2C1->MCS & (1U<<BUSY));
  I2C1->MCS = (1U<<ACK) | (1U<<RUN);
  return I2C1->MDR;
}

extern uint8_t I2C_ReceiveNAck(void)
{
  while(I2C1->MCS & (1U<<BUSY));
  I2C1->MCS = (1U<<ACK) | (1U<<RUN);
  return I2C1->MDR;
}

extern void I2C_StopReceive(void)
{
  while(I2C1->MCS & (1U<<BUSY));
  I2C1->MCS = (1U<<STOP) | (1U<<RUN) ;
}

extern void I2C_StopTransmit(void)
{
  while(I2C1->MCS & (1U<<BUSY));
  I2C1->MCS = (1U<<STOP);
  uint16_t i=0;
  for(i=0; i<15000; i++);
}