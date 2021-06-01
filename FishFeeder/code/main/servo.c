// Authors 
// Hayato Nakamura, Yang Hang Liu, 9/22

// include header file
#include "servo.h"

// global vars
extern bool do_feed;        // control when to activate servo

void mcpwm_example_gpio_initialize(void) {
    printf("initializing mcpwm servo control gpio......\n");
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, 12);    //Set GPIO 12 as PWM0A
}

// this function calcute pulse width for per degree rotation //
uint32_t servo_per_degree_init(uint32_t degree_of_rotation) {
  // cal_pulsewidth: the angle in degree to which servo has to rotate
  uint32_t cal_pulsewidth = 0;
  cal_pulsewidth = (SERVO_MIN_PULSEWIDTH + (((SERVO_MAX_PULSEWIDTH - SERVO_MIN_PULSEWIDTH) * (degree_of_rotation)) / (SERVO_MAX_DEGREE)));
  return cal_pulsewidth;
}

// Configure MCPWM module //
void mcpwm_example_servo_control(void *arg) {
  int i;
  uint32_t angle, count;
  //1. mcpwm gpio initialization
  mcpwm_example_gpio_initialize();

  //2. initial mcpwm configuration
  printf("Configuring Initial Parameters of mcpwm......\n");
  mcpwm_config_t pwm_config;
  pwm_config.frequency = 50;    //frequency = 50Hz, i.e. for every servo motor time period should be 20ms
  pwm_config.cmpr_a = 0;        //duty cycle of PWMxA = 0
  pwm_config.cmpr_b = 0;        //duty cycle of PWMxb = 0
  pwm_config.counter_mode = MCPWM_UP_COUNTER;
  pwm_config.duty_mode = MCPWM_DUTY_MODE_0;
  mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &pwm_config);    //Configure PWM0A & PWM0B with above settings
  while (1) {
    if (do_feed) {
      for (i=0; i<3; i++) { // rotate back and forth three times
        for (count = 0; count < SERVO_MAX_DEGREE; count++) {
          printf("Angle of rotation: %d\n", count);
          angle = servo_per_degree_init(count);
          printf("pulse width: %dus\n", angle);
          mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, angle);
          vTaskDelay(4);     //Add delay, since it takes time for servo to rotate, generally 100ms/60degree rotation at 5V
        }
      }
      do_feed = false;
    } else {
      printf("waiting for next feeding time!\n");
    }
  }
}
