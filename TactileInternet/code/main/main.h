/* Authors: Hayato Nakamura
            Yang Hang Liu
            Arnaud Harmange 
*/
#ifndef MAIN_H
#define MAIN_H 

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <stdbool.h>
#include "esp_types.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "driver/adc.h"
#include "sdkconfig.h"
#include "esp_attr.h"
#include "esp_adc_cal.h"

// ADC related
#define DEFAULT_VREF    1100        //Use adc2_vref_to_gpio() to obtain a better estimate
#define NO_OF_SAMPLES   100          //Multisampling
#define INTERVAL        2000        //Sampling interval in ms

// thermistor related
#define INPUT_VOLTAGE 3300   // mV
#define KNOWN_REGISTOR 1000  // Ohms

// adc related vars
static esp_adc_cal_characteristics_t *adc_chars;
static const adc_channel_t thermistor_channel = ADC_CHANNEL_4; // GPIO32 (A7) for thermistor
static const adc_channel_t ultrasonic_channel = ADC_CHANNEL_5; // GPIO33 (A9) for ultrasonic sensor
static const adc_channel_t ir_channel = ADC_CHANNEL_6;         // GPIO34 (A2) for IR Rangefinder
static const adc_atten_t atten = ADC_ATTEN_DB_11;
static const adc_unit_t unit = ADC_UNIT_1;

// voltage vars for each sensor types
uint32_t thermistor_voltage;
uint32_t ultrasonic_voltage;
uint32_t ir_voltage;

// stores engineering unit distance
float ir_detected_dist;
float ultrasonic_detected_dist;

// thermistor related vars
float thermistor_reg;
float log_reg, T, Tc, Tf;
// Coeffs for Steinhart-Hart Equation to calculate temperature
// float c1 = 0.001125308852122, c2 = 0.000234711863267, c3 = 0.000000085663516;
float c1 = 0.001225308852122, c2 = 0.000234711863267, c3 = 0.000000085663516;
// timer for log
float timer = 0.0;

// initialize all modules
void init();
// efuse status
static void check_efuse(void);
static void print_char_val_type(esp_adc_cal_value_t);

// typedef
typedef struct {
    int flag;
} timer_event_t;

#endif