#ifndef MAIN_H
#define MAIN_H 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h> 
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "driver/timer.h"
#include "esp_err.h"
#include "esp_types.h"
#include "soc/uart_reg.h"

// Timer related
#define TIMER_DIVIDER         16     //  Hardware timer clock divider
#define TIMER_SCALE           (TIMER_BASE_CLK / TIMER_DIVIDER)  // to seconds
#define TIMER_INTERVAL_SEC    (1)    // Sample test interval for the first timer
#define TIMER_INTERVAL_2_SEC  (2)
#define TIMER_INTERVAL_10_SEC (10)
#define TEST_WITH_RELOAD      1      // Testing will be done with auto reload

// Define each timeout
#define ELECTION_TIMEOUT      5       // election finish
#define LEADER_TIMEOUT        10      // start leader election
#define HEARTBEAT_TIMEOUT     3       // leader must send alive msg within this time.

// Define Device Unique ID
#define MYID                  5

// typedef struct {
//     int flag;     // flag for enabling stuff in main code
// } timer_event_t;

typedef enum {			// Set of states enumerated
    ELECTION_STATE,     // 0 (associated number for udp msg)
    NON_LEADER_STATE,   // 1
    LEADER_STATE        // 2 
} state_e;

typedef enum {			// Set of events enumerated
	NO_EVENT,
    BECOME_NON_LEADER_EVENT,
    BECOME_LEADER_EVENT,
    ELECTION_TIMEOUT_EVENT,
    LEADER_TIMEOUT_EVENT,
    HEARTBEAT_TIMEOUT_EVENT,
    MESSAGE_RECEIVE_EVENT
} event_e;

#endif