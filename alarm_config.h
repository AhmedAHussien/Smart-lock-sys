#ifndef _ALARM_CONFIG_H_
#define _ALARM_CONFIG_H_

//SYSTEM CLOCK GATING BITS
#define RCGC_BUZZER   0
#define RCGC_LED_G    5
#define RCGC_LED_R    5

/******** IMPORTANT *********/
//When changing one device's port to another make sure to change the system clock gating bits accordingly
#define BUZZER_PORT   GPIOA
#define LED_G_PORT    GPIOF
#define LED_R_PORT    GPIOF

#define BUZZER_PIN    5
#define LED_R_PIN     4
#define LED_G_PIN     3

#endif
