
#ifndef _I2C_H_
#define _I2C_H_

#include <stdint.h>

#define EEPROM_WRITE_ADDRESS		0xA0
#define EEPROM_READ_ADDRESS			0xA1
#define errCOULD_NOT_GENERATE_START   0
#define SUCCESS                       1


extern void I2C_Init(void);
extern uint8_t I2C_StartTransmit(uint16_t eepromAddress);
extern uint8_t I2C_StartReceive(uint16_t eepromAddress);
extern void I2C_Transmit(uint8_t data);
extern uint8_t I2C_ReceiveAck(void);
extern uint8_t I2C_ReceiveNAck(void);
extern void I2C_StopReceive(void);
extern void I2C_StopTransmit(void);


#endif