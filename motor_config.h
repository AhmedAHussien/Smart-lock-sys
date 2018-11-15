#ifndef _MOTOR_CONFIG_H_
#define _MOTOR_CONFIG_H_

#define DEBUG_MODE              0


//Refer to the datasheet before making any changes
//if the pwm generator needs to be changed, make sure to make the nescessary changes in motor.c file

//SYSTEM CLOCK GATING
#define RCGC_MOTOR_PWM_MODULE   1
#define RCGC_ENABLE_A           5
#define RCGC_INPUT_1            1
#define RCGC_INPUT_2            1

#define PWM_MODULE_BLOCK_NO        6
#define PWM_GENERATOR_NO           3

#define PORTCONTROL_PMC         8

#define ENABLE_A_PORT           GPIOF
#define INPUT_1_PORT            GPIOB
#define INPUT_2_PORT            GPIOB


#define ENABLE_A_PIN            2
#define INPUT_1_PIN             0
#define INPUT_2_PIN             1

#define SYSTEM_CLOCK_FREQ       16000000UL
#define PWM_FREQUENCY           3000
#define PWM_DUTY_CYCLE          30

#endif
