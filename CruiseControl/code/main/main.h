/*
  Hayato Nakamura
  Yang Hang Liu
  Arnaud Harmange
*/

#ifndef MAIN_H
#define MAIN_H 

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include "esp_types.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_attr.h"
#include "esp_adc_cal.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"
#include "driver/mcpwm.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "driver/uart.h"
#include "driver/periph_ctrl.h"
#include "driver/timer.h"
#include "driver/rmt.h"
#include "driver/i2c.h"
#include "soc/mcpwm_periph.h"
#include "soc/rmt_reg.h"
#include <sys/time.h>
#include "sdkconfig.h"

// uart related
#define EX_UART_NUM UART_NUM_0

//You can get these value from the datasheet of servo you use, in general pulse width varies between 1000 to 2000 mocrosecond
#define SERVO_MIN_PULSEWIDTH 1000 //Minimum pulse width in microsecond 1000
#define SERVO_MAX_PULSEWIDTH 2000 //Maximum pulse width in microsecond 2000
#define SERVO_MAX_DEGREE 90 //Maximum angle in degree upto which servo can rotate

#define STEERING_SERVO_MIN_PULSEWIDTH 1000 //Minimum pulse width in microsecond
#define STEERING_SERVO_MAX_PULSEWIDTH 1800 //Maximum pulse width in microsecond
#define STEERING_SERVO_MAX_DEGREE 90 //Maximum angle in degree upto which servo can rotate

#define DEFAULT_VREF    1100        //Use adc2_vref_to_gpio() to obtain a better estimate
#define NO_OF_SAMPLES   2          //Multisampling

// Timer related
#define TIMER_DIVIDER         16    //  Hardware timer clock divider
#define TIMER_SCALE           (TIMER_BASE_CLK / TIMER_DIVIDER)  // to seconds
#define TIMER_INTERVAL_SEC    (1)    // Sample test interval for the first timer
#define TEST_WITH_RELOAD      1     // Testing will be done with auto reload

// Ultrasonic Range Sensor
#define RMT_TX_CHANNEL          1 /* RMT channel for transmitter */
#define RMT_TX_GPIO_NUM         PIN_TRIGGER /* GPIO number for transmitter signal */
#define RMT_RX_CHANNEL          0 /* RMT channel for receiver */
#define RMT_RX_GPIO_NUM         PIN_ECHO /* GPIO number for receiver */
#define RMT_CLK_DIV             100 /* RMT counter clock divider */
#define RMT_TX_CARRIER_EN       0 /* Disable carrier */
#define rmt_item32_tIMEOUT_US   9500 /*!< RMT receiver timeout value(us) */
#define RMT_TICK_10_US          (80000000/RMT_CLK_DIV/100000) /* RMT counter value for 10 us.(Source clock is APB clock) */
#define ITEM_DURATION(d)        ((d & 0x7fff)*10/RMT_TICK_10_US)
#define PIN_TRIGGER              27
#define PIN_ECHO                 12

// 14-Segment Display related
#define SLAVE_ADDR                         0x70 // alphanumeric address
#define OSC                                0x21 // oscillator cmd
#define HT16K33_BLINK_DISPLAYON            0x01 // Display on cmd
#define HT16K33_BLINK_OFF                  0    // Blink off cmd
#define HT16K33_BLINK_CMD                  0x80 // Blink cmd
#define HT16K33_CMD_BRIGHTNESS             0xE0 // Brightness cmd

// I2C related
#define I2C_EXAMPLE_MASTER_SCL_IO          22   // gpio number for i2c clk
#define I2C_EXAMPLE_MASTER_SDA_IO          23   // gpio number for i2c data
#define I2C_EXAMPLE_MASTER_NUM             I2C_NUM_0  // i2c port
#define I2C_EXAMPLE_MASTER_TX_BUF_DISABLE  0    // i2c master no buffer needed
#define I2C_EXAMPLE_MASTER_RX_BUF_DISABLE  0    // i2c master no buffer needed
#define I2C_EXAMPLE_MASTER_FREQ_HZ         100000     // i2c master clock freq
#define WRITE_BIT                          I2C_MASTER_WRITE // i2c master write
#define READ_BIT                           I2C_MASTER_READ  // i2c master read
#define ACK_CHECK_EN                       true // i2c master will check ack
#define ACK_CHECK_DIS                      false// i2c master will not check ack
#define ACK_VAL                            0x00 // i2c ack value
#define NACK_VAL                           0xFF // i2c nack value
#define BUF_SIZE                           1024


// prototype def
int take_input(char* ascii_input);
int ascii2segment(char* ascii_input, uint16_t* led_segment);

// typedef
typedef struct {
    int flag;     // flag for enabling stuff in main code
} timer_event_t;

#endif