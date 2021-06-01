// Authors 
// Hayato Nakamura, Yang Hang Liu 9/22

// include header file
#include "main.h"
#include "servo.h"
#include "display.h"
#include "helper.h"

// global vars
xQueueHandle timer_queue;   // for timer-based events
int stopwatch;              // for keeping track of time
bool do_feed;               // control when to activate servo

void init() {	
  // initialize global vars
  stopwatch = FEEDING_INTERFVAL; // initial feeding interval is 3hr in second notation
  stopwatch = 10;                // for demonstration purpose, feed in 10 secs.
  do_feed = false;

  // initialize i2c
  i2c_example_master_init();
  i2c_scanner();
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
  uart_param_config(EX_UART_NUM, &uart_config);
  uart_set_pin(EX_UART_NUM, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
  uart_driver_install(EX_UART_NUM, BUF_SIZE * 2, 0, 0, NULL, 0);

  // initialize timer 
  // Create a FIFO queue for timer-based
  timer_queue = xQueueCreate(10, sizeof(timer_event_t));
}

void app_main()
{
  init();				

  // create tasks
  xTaskCreate(display_state, "display_state", 1024*2, NULL, configMAX_PRIORITIES, NULL);
  xTaskCreate(timer_evt_task, "timer_evt_task", 1024*2, NULL, configMAX_PRIORITIES, NULL);
  xTaskCreate(mcpwm_example_servo_control, "mcpwm_example_servo_control", 1024*2, NULL, configMAX_PRIORITIES, NULL);

  // Initiate alarm using timer API
  alarm_init();
}	