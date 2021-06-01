// Authors 
// Hayato Nakamura, 9/22

#ifndef DISPLAY_H
#define DISPLAY_H 

#include "main.h"

// convert current countdown to ascii and display 
void display_state();

// ISR handler //
void IRAM_ATTR timer_group0_isr(void *para);

// Initialize timer 0 in group 0 for 1 sec alarm interval and auto reload //
void alarm_init();

// The timer task
void timer_evt_task(void *arg);

// initialize i2c
void i2c_example_master_init();

// Utility function to test for I2C device address -- not used in deploy
int testConnection(uint8_t devAddr, int32_t timeout);

// Utility function to scan for i2c device
void i2c_scanner();

// Alphanumeric Functions //
// Turn on oscillator for alpha display
int alpha_oscillator();

// Set blink rate to off
int no_blink();

// Set Brightness
int set_brightness_max(uint8_t val);

#endif
