// Authors 
// Hayato Nakamura, 9/22

#ifndef SERVO_H
#define SERVO_H 

#include "main.h"

// servo initialization
void mcpwm_example_gpio_initialize(void);

// this function calcute pulse width for per degree rotation //
uint32_t servo_per_degree_init(uint32_t degree_of_rotation);

// Configure MCPWM module //
void mcpwm_example_servo_control(void *arg);

#endif
