#include "main.h"
#include "udp.h"
#include "ir.h"

int election_timer = 0;  // timer to keep track ELECTION_TIMEOUT_EVENT
int leader_timer = LEADER_TIMEOUT-1;    // timer to keep track LEADER_TIMEOUT_EVENT (start election when join)
int heartbeat_timer = 0; // timer to keep track HEARTBEAT_TIMEOUT_EVENT
xQueueHandle timer_queue; 
state_e state = NON_LEADER_STATE;	// Starting state
state_e next_state;
event_e event = NO_EVENT;
int minID = MYID;        // minID is equal to MYID at initial state (potential leader)
char myColor = (char) COLOR;
char voterID;
bool voted = false;
bool vote_ready = false;

// ISR handler
void IRAM_ATTR timer_group0_isr(void *para) {

    // Prepare basic event data, aka set flag
    timer_event_t evt;
    evt.flag = 1;

    // Clear the interrupt, Timer 0 in group 0
    TIMERG0.int_clr_timers.t0 = 1;

    // After the alarm triggers, we need to re-enable it to trigger it next time
    TIMERG0.hw_timer[TIMER_0].config.alarm_en = TIMER_ALARM_EN;

    // Send the event data back to the main program task
    xQueueSendFromISR(timer_queue, &evt, NULL);
}

// // Initialize timer 0 in group 0 for 1 sec alarm interval and auto reload
static void main_alarm_init() {
    /* Select and initialize basic parameters of the timer */
    timer_config_t config;
    config.divider = TIMER_DIVIDER;
    config.counter_dir = TIMER_COUNT_UP;
    config.counter_en = TIMER_PAUSE;
    config.alarm_en = TIMER_ALARM_EN;
    config.intr_type = TIMER_INTR_LEVEL;
    config.auto_reload = TEST_WITH_RELOAD;
    timer_init(TIMER_GROUP_0, TIMER_0, &config);

    // Timer's counter will initially start from value below
    timer_set_counter_value(TIMER_GROUP_0, TIMER_0, 0x00000000ULL);

    // Configure the alarm value and the interrupt on alarm
    timer_set_alarm_value(TIMER_GROUP_0, TIMER_0, TIMER_INTERVAL_SEC * TIMER_SCALE);
    timer_enable_intr(TIMER_GROUP_0, TIMER_0);
    timer_isr_register(TIMER_GROUP_0, TIMER_0, timer_group0_isr,
        (void *) TIMER_0, ESP_INTR_FLAG_IRAM, NULL);

    // Start timer
    timer_start(TIMER_GROUP_0, TIMER_0);

    timer_queue = xQueueCreate(10, sizeof(timer_event_t));
}

// // GPIO init for Onboard LED
// static void led_init() {
//     gpio_pad_select_gpio(13);
//     gpio_set_direction(13, GPIO_MODE_OUTPUT);
// }

// // Configure parameters of the UART driver, communication pins and install the driver
// static void uart_init() {
//   uart_config_t uart_config = {
//       .baud_rate = 115200,
//       .data_bits = UART_DATA_8_BITS,
//       .parity    = UART_PARITY_DISABLE,
//       .stop_bits = UART_STOP_BITS_1,
//       .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
//       .source_clk = UART_SCLK_APB,
//   };
//   uart_param_config(UART_NUM_0, &uart_config);
//   uart_set_pin(UART_NUM_0, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
//   uart_driver_install(UART_NUM_0, 1024 * 2, 0, 0, NULL, 0);
// }

// initialize all components
void init() {
  // uart_init();
  main_alarm_init();
  // led_init();

  // ir initialization
  ir_init();

  // udp module initialization
  ESP_ERROR_CHECK( nvs_flash_init() );
  initialise_wifi();
  wait_for_ip();
}

// control onboard LED based on state
void onboard_led_task(){
  while(1) {
    if (state == NON_LEADER_STATE) {
      gpio_set_level(13,0);
    } else if (state == LEADER_STATE) {
      gpio_set_level(13,1);
    } else if (state == ELECTION_STATE) {
      for (int i = 0; i < 5; i++) {
        gpio_set_level(13,1);
        vTaskDelay(200 / portTICK_PERIOD_MS);
        gpio_set_level(13,0);
        vTaskDelay(200 / portTICK_PERIOD_MS);
      }
    }
    vTaskDelay(200 / portTICK_PERIOD_MS);
  }
}

// The main task of this example program
static void timer_evt_task(void *arg) {
    while (1) {
        // Create dummy structure to store structure from queue
        timer_event_t evt;

        // Transfer from queue
        xQueueReceive(timer_queue, &evt, portMAX_DELAY);
        printf("========\n");
        printf("%d %d %d\n", election_timer, leader_timer, heartbeat_timer);
        printf("========\n");

        // Do something if triggered!
        if (evt.flag == 1) {
            if (state == ELECTION_STATE) {
              if (election_timer >= ELECTION_TIMEOUT) {
                election_timer = 1;
                event = ELECTION_TIMEOUT_EVENT;
              } else { 
                election_timer++;
              }
              voted = false;
            } else {
              election_timer = 1;
            }
            if (leader_timer >= LEADER_TIMEOUT) {
              leader_timer = 1;
              event = LEADER_TIMEOUT_EVENT;
            } else {
              leader_timer++;
            }
            if (state == LEADER_STATE) {
              if (heartbeat_timer >= HEARTBEAT_TIMEOUT) {
                heartbeat_timer = 1;
                event = HEARTBEAT_TIMEOUT_EVENT;
              } else {
                heartbeat_timer++;
              }
            } else {
              heartbeat_timer = 1;
            }
        }
    }
}

void app_main(void) {
  init();
  xTaskCreate(timer_evt_task, "timer_evt_task", 1024*2, NULL, configMAX_PRIORITIES, NULL);
  xTaskCreate(onboard_led_task, "onboard_led_task", 1024*2, NULL, configMAX_PRIORITIES, NULL);

  // udp
  xTaskCreate(udp_client_task, "udp_client", 1024*2, NULL, configMAX_PRIORITIES, NULL);
  xTaskCreate(udp_server_task, "udp_server", 1024*2, NULL, configMAX_PRIORITIES, NULL);

  // ir
  // Create tasks for receive, send, set gpio, and button
  xTaskCreate(recv_task, "uart_rx_task", 1024*4, NULL, configMAX_PRIORITIES, NULL);
  // xTaskCreate(send_task, "uart_tx_task", 1024*2, NULL, configMAX_PRIORITIES, NULL);
  xTaskCreate(led_task, "set_traffic_task", 1024*2, NULL, configMAX_PRIORITIES, NULL);
  // xTaskCreate(id_task, "set_id_task", 1024*2, NULL, configMAX_PRIORITIES, NULL);
  xTaskCreate(button_task, "button_task", 1024*2, NULL, configMAX_PRIORITIES, NULL);
  // xTaskCreate(button_task_1, "button_task_1", 1024*2, NULL, configMAX_PRIORITIES, NULL);
  // xTaskCreate(button_task_2, "button_task_2", 1024*2, NULL, configMAX_PRIORITIES, NULL);
}