#ifndef LEDC_H
#define LEDC_H 

#include "main.h"
#include "driver/ledc.h"
#include "driver/uart.h"
#include "driver/gpio.h"

#define LEDC_HS_TIMER          LEDC_TIMER_0
#define LEDC_HS_MODE           LEDC_HIGH_SPEED_MODE
#define LEDC_HS_CH0_GPIO       (18)
#define LEDC_HS_CH0_CHANNEL    LEDC_CHANNEL_0
#define LEDC_LS_TIMER          LEDC_TIMER_1
#define LEDC_LS_MODE           LEDC_LOW_SPEED_MODE

#define LEDC_TEST_CH_NUM       (1)
#define LEDC_TEST_DUTY         (4000)
#define LEDC_TEST_FADE_TIME    (3000)

#define LEDC_DUTY_MAX          (8192)


/*
  Duty resolution is 13, so the range of duty setting is [0, 8192]
*/

void ledc_0();

void ledc_init();

#endif