// Authors 
/*
  Hayato Nakamura
  Yang Hang Liu
  Arnaud Harmange
*/

#ifndef DISPLAY_H
#define DISPLAY_H 

#include "main.h"
#include "helper.h"

// convert current countdown to ascii and display 
void display_state();

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
