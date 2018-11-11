/*
 * utils.h
 *
 * Created: 13-Jan-18 5:32:23 PM
 *  Author: Ahmed Hessein
 */ 


#ifndef UTILS_H_
#define UTILS_H_

#define SET_REG(Reg) (Reg |= 0xFF)
#define CLEAR_REG(Reg) (Reg &= 0x00)
#define ASSIGN_REG(Reg, Value) (Reg = Value)
#define TOGGLE_REG(Reg) (Reg ^= 0xFF)

#define SET_BIT(Reg, Bit) (Reg |= (1 << Bit))
#define CLEAR_BIT(Reg, Bit) (Reg &= ~(1<< Bit))
#define TOGGLE_BIT(Reg, Bit) (Reg ^= (1<< Bit))
#define GET_BIT(Reg, Bit) ((Reg >> Bit) & 1)

#endif /* UTILS_H_ */