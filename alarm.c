#include "alarm_config.h"
#include "alarm.h"
#include "TM4C123GH6PM.h"


extern void Alarm_Init(void)
{

  SYSCTL->RCGCGPIO |= (1U<<RCGC_BUZZER) | (1U<<RCGC_LED_G) | (1U<<RCGC_LED_R);

  BUZZER_PORT->DEN |= (1U<<BUZZER_PIN);
  BUZZER_PORT->DIR |= (1U<<BUZZER_PIN);

  LED_G_PORT->DEN |= (1U<<LED_G_PIN);
  LED_G_PORT->DIR |= (1U<<LED_G_PIN);

  LED_R_PORT->DEN |= (1U<<LED_R_PIN);
  LED_R_PORT->DIR |= (1U<<LED_R_PIN);

}

extern void Alarm_GreenLedOn(void)
{
  LED_G_PORT->DATA_BITS[(1U<<LED_G_PIN)] = (1U<<LED_G_PIN);
}

extern void Alarm_GreenLedOff(void)
{
  LED_G_PORT->DATA_BITS[(1U<<LED_G_PIN)] = 0;
}

extern void Alarm_RedLedOn(void)
{
  LED_R_PORT->DATA_BITS[(1U<<LED_R_PIN)] = (1U<<LED_R_PIN);
}

extern void Alarm_RedLedOff(void)
{
  LED_R_PORT->DATA_BITS[(1U<<LED_R_PIN)] = 0;
}

extern void Alarm_BuzzerOn(void)
{
  BUZZER_PORT->DATA_BITS[(1U<<BUZZER_PIN)] = (1U<<BUZZER_PIN);
}

extern void Alarm_BuzzerOff(void)
{
  BUZZER_PORT->DATA_BITS[(1U<<BUZZER_PIN)] = 0;
}
