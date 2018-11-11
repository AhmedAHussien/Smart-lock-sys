/*
 * keypad_config.h
 *
 * Created: 3/29/2018 8:35:24 PM
 *  Author: Peter
 */ 


#ifndef KEYPAD_CONFIG_H_
#define KEYPAD_CONFIG_H_

// row -- output
#define  ROW0           1
#define  ROW0_PORT      GPIOF

#define  ROW1           3
#define  ROW1_PORT      GPIOE

#define  ROW2           2
#define  ROW2_PORT      GPIOE

#define  ROW3           1
#define  ROW3_PORT      GPIOE


// col as input
#define  COL0           3
#define  COL0_PORT      GPIOD

#define  COL1           2
#define  COL1_PORT      GPIOD

#define  COL2           1
#define  COL2_PORT      GPIOD

#define  COL3           0
#define  COL3_PORT      GPIOD


#endif /* KEYPAD_CONFIG_H_ */