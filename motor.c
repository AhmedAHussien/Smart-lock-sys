#include "motor_config.h"
#include "motor.h"
#include "TM4C123GH6PM.h"


extern void Motor_Init(void)
{
  //PF2 -->> M1PWM6  Controlled by Module 1 PWM Gen 3


  SYSCTL->RCGCGPIO |= (1U<<RCGC_ENABLE_A) | (1U<<RCGC_INPUT_1) | (1U<<RCGC_INPUT_2);
  ENABLE_A_PORT->AFSEL |= (1U<<ENABLE_A_PIN);
  ENABLE_A_PORT->PCTL |= (5<<PORTCONTROL_PMC);
  ENABLE_A_PORT->DIR |= (1U<<ENABLE_A_PIN);
  ENABLE_A_PORT->DEN |= (1U<<ENABLE_A_PIN);

  INPUT_1_PORT->DIR |= (1U<<INPUT_1_PIN);
  INPUT_1_PORT->DEN |= (1U<<INPUT_1_PIN);

  INPUT_2_PORT->DIR |= (1U<<INPUT_2_PIN);
  INPUT_2_PORT->DEN |= (1U<<INPUT_2_PIN);

  //PWM Initialization
  SYSCTL->RCGCPWM |= (1U<<RCGC_MOTOR_PWM_MODULE);   //enable clock gating to PWM module 1
  SYSCTL->RCC |= (1U<<20);
  //drive pwmA low when counting down, drive pwmA high when counting up (on cmpA)
  PWM1->_3_GENA = (0x2<<6) | (0x3<<4);
  //for 3kHz PWM frequency, system clock 16MHz, sysclk divided by 2: LOAD = 2665 (0xA69)
  PWM1->_3_LOAD = (((SYSTEM_CLOCK_FREQ/2)/PWM_FREQUENCY)-1);
  //for 35% duty cycle: CMPA = 1359 (0x54f), voltage supply is 8.5, motor's full speed 2400RPM @ 6V; for half speed -> (3+1.4)/8.5 = 0.51
  PWM1->_3_CMPA = (((SYSTEM_CLOCK_FREQ/2)/PWM_FREQUENCY)-1)*PWM_DUTY_CYCLE/100;
  //Enables PWM6
  PWM1->ENABLE = (1U<<PWM_MODULE_BLOCK_NO);
  //Inverts PWM6
  PWM1->INVERT = (1U<<PWM_MODULE_BLOCK_NO);
  //enable GLOBALSYNC 3
  PWM1->CTL = (1U<<PWM_GENERATOR_NO);
  // sets mode for count up/down
  PWM1->_3_CTL = (1U<<1);



}

extern void Motor_RunForward(void)
{
  INPUT_2_PORT->DATA_BITS[(1U<<INPUT_2_PIN)] = 0;
  INPUT_1_PORT->DATA_BITS[(1U<<INPUT_1_PIN)] = (1U<<INPUT_1_PIN);
  PWM1->_3_CTL |= (1U<<0);   //Generates PWM

}

extern void Motor_RunReverse(void)
{
  INPUT_1_PORT->DATA_BITS[(1U<<INPUT_1_PIN)] = 0;
  INPUT_2_PORT->DATA_BITS[(1U<<INPUT_2_PIN)] = (1U<<INPUT_2_PIN);
  PWM1->_3_CTL |= (1U<<0); //Generates PWM

}

extern void Motor_Stop(void)
{
  INPUT_1_PORT->DATA_BITS[(1U<<INPUT_1_PIN)] = 0;
  INPUT_2_PORT->DATA_BITS[(1U<<INPUT_2_PIN)] = 0;
  PWM1->_3_CTL &=~ (1U<<0); //Stops PWM


}
