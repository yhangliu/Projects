#include "main.h"
#include "ledc.h"

// global vars
extern char led_intensity;

ledc_channel_config_t ledc_channel[LEDC_TEST_CH_NUM] = {
  {
      .channel    = LEDC_HS_CH0_CHANNEL,
      .duty       = 0,
      .gpio_num   = LEDC_HS_CH0_GPIO,
      .speed_mode = LEDC_HS_MODE,
      .hpoint     = 0,
      .timer_sel  = LEDC_HS_TIMER
  },
};

/*
  Duty resolution is 13, so the range of duty setting is [0, 8192]
*/

void ledc_0() {
  // control ledc_channel_0
  // takes user input from console to change led intensity 
  uint8_t *data = (uint8_t *) malloc(1024);
  int duty_value = (int)(LEDC_DUTY_MAX*0.9);;

  while (1) {
    if (led_intensity == '0') duty_value = (int)(LEDC_DUTY_MAX*0.0);
    else if (led_intensity == '1') duty_value = (int)(LEDC_DUTY_MAX*0.1);
    else if (led_intensity == '2') duty_value = (int)(LEDC_DUTY_MAX*0.2);
    else if (led_intensity == '3') duty_value = (int)(LEDC_DUTY_MAX*0.3);
    else if (led_intensity == '4') duty_value = (int)(LEDC_DUTY_MAX*0.4);
    else if (led_intensity == '5') duty_value = (int)(LEDC_DUTY_MAX*0.5);
    else if (led_intensity == '6') duty_value = (int)(LEDC_DUTY_MAX*0.6);
    else if (led_intensity == '7') duty_value = (int)(LEDC_DUTY_MAX*0.7);
    else if (led_intensity == '8') duty_value = (int)(LEDC_DUTY_MAX*0.8);
    else if (led_intensity == '9') duty_value = (int)(LEDC_DUTY_MAX*0.9);
    else duty_value = (int)(LEDC_DUTY_MAX*0.9);

    // printf("Blue LEDC set duty = %d\n", duty_value);
    ledc_set_duty(ledc_channel[0].speed_mode, ledc_channel[0].channel, duty_value);
    ledc_update_duty(ledc_channel[0].speed_mode, ledc_channel[0].channel);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void ledc_init() {
    led_intensity = '0';
  /* Configure parameters of the UART driver,
    * communication pins and install the driver */
  uart_config_t uart_config = {
      .baud_rate = 115200,
      .data_bits = UART_DATA_8_BITS,
      .parity    = UART_PARITY_DISABLE,
      .stop_bits = UART_STOP_BITS_1,
      .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
      .source_clk = UART_SCLK_APB,
  };
  uart_param_config(UART_NUM_0, &uart_config);
  uart_set_pin(UART_NUM_0, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
  uart_driver_install(UART_NUM_0, 1024 * 2, 0, 0, NULL, 0);

  int ch;

  /*
    * Prepare and set configuration of timers
    * that will be used by LED Controller
    */
  ledc_timer_config_t ledc_timer = {
      .duty_resolution = LEDC_TIMER_13_BIT, // resolution of PWM duty
      .freq_hz = 5000,                      // frequency of PWM signal
      .speed_mode = LEDC_LS_MODE,           // timer mode
      .timer_num = LEDC_LS_TIMER,            // timer index
      .clk_cfg = LEDC_AUTO_CLK,              // Auto select the source clock
  };
  // Set configuration of timer0 for high speed channels
  ledc_timer_config(&ledc_timer);
  // Prepare and set configuration of timer1 for low speed channels
  ledc_timer.speed_mode = LEDC_HS_MODE;
  ledc_timer.timer_num = LEDC_HS_TIMER;
  ledc_timer_config(&ledc_timer);

  // Set LED Controller with previously prepared configuration
  for (ch = 0; ch < LEDC_TEST_CH_NUM; ch++) {
      ledc_channel_config(&ledc_channel[ch]);
  }

  // Initialize fade service.
  ledc_fade_func_install(0);
}