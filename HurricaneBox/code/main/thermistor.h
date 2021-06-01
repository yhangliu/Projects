#ifndef THERMISTOR_H
#define THERMISTOR_H 

#include "main.h"
#include <math.h>
#include <unistd.h>
#include <stdbool.h>
#include "esp_types.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "driver/adc.h"
#include "driver/i2c.h"
#include "driver/periph_ctrl.h"
#include "driver/timer.h"
#include "sdkconfig.h"
#include "esp_attr.h"
#include "esp_adc_cal.h"

// uart related
#define EX_UART_NUM UART_NUM_0

// ADC related
#define DEFAULT_VREF    1100        //Use adc2_vref_to_gpio() to obtain a better estimate
#define NO_OF_SAMPLES   20          //Multisampling

// thermistor related
#define INPUT_VOLTAGE 3300   // mV
#define KNOWN_REGISTOR 1000 // Ohms

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
void check_efuse(void);

void print_char_val_type(esp_adc_cal_value_t val_type);

void thermistor_init();

void thermistor();

#endif