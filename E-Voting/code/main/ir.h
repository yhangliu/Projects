/*
  Hayato Nakamura
  Yang Hang Liu
  Arnaud Harmange
*/

#ifndef IR_H
#define IR_H

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/rmt.h"
#include "soc/rmt_reg.h"
#include "driver/uart.h"
#include "driver/periph_ctrl.h"
#include "driver/timer.h"
#include "soc/uart_reg.h"

// RMT definitions
#define RMT_TX_CHANNEL    1     // RMT channel for transmitter
#define RMT_TX_GPIO_NUM   25    // GPIO number for transmitter signal -- A1
#define RMT_CLK_DIV       100   // RMT counter clock divider
#define RMT_TICK_10_US    (80000000/RMT_CLK_DIV/100000)   // RMT counter value for 10 us.(Source clock is APB clock)
#define rmt_item32_tIMEOUT_US   9500     // RMT receiver timeout value(us)

// UART definitions
#define UART_TX_GPIO_NUM 26 // A0
#define UART_RX_GPIO_NUM 34 // A2
#define BUF_SIZE (1024)

// Hardware interrupt definitions
#define GPIO_INPUT_IO_1       4 // A5 -> send uart signal
#define GPIO_INPUT_IO_2       33 //36 // A4 -> cycle led
#define ESP_INTR_FLAG_DEFAULT 0
#define GPIO_INPUT_PIN_SEL    1ULL<<GPIO_INPUT_IO_1

// LED Output pins definitions
#define BLUEPIN   14
#define GREENPIN  32
#define REDPIN    15
#define ONBOARD   13

#define TIMER_DIVIDER         16    //  Hardware timer clock divider
#define TIMER_SCALE           (TIMER_BASE_CLK / TIMER_DIVIDER)  // to seconds
#define TIMER_INTERVAL_2_SEC  (2)
#define TIMER_INTERVAL_10_SEC (10)
#define TEST_WITH_RELOAD      1     // Testing will be done with auto reload

// Default ID/color
#define ID 3
#define COLOR 'R'

typedef struct {
    int flag;     // flag for enabling stuff in main code
} timer_event_t;

// prototype def
void IRAM_ATTR gpio_isr_handler(void* arg);

// ISR handler
void IRAM_ATTR timer_group1_isr(void *para);

// Utilities ///////////////////////////////////////////////////////////////////
// Checksum
char genCheckSum(char *p, int len);

bool checkCheckSum(uint8_t *p, int len);

// Init Functions //////////////////////////////////////////////////////////////
// RMT tx init
void rmt_tx_init();

// Configure UART
void uart_init();

// GPIO init for LEDs
void led_init();

// Configure timer
void alarm_init();

// Button interrupt init
void button_init();

// Task Functions //////////////////////////////////////////////////////////////
// Button task -- rotate through myIDs
void button_task();
void button_task_1();
void button_task_2();

// Send task -- sends payload | Start | myID | Start | myID
void send_task();

// Receives task -- looks for Start byte then stores received values
void recv_task();

// LED task to light LED based on traffic state
void led_task();

// LED task to blink onboard LED based on ID
void id_task();

// Timer task -- R (10 seconds), G (10 seconds), Y (2 seconds)
// void timer_evt_task(void *arg);

void ir_init();


#endif 