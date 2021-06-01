/*
  Hayato Nakamura
  Yang Hang Liu
  Arnaud Harmange
*/

#include "main.h"
#include "udp.h"
#include "display.h"

esp_adc_cal_characteristics_t *adc_chars;
const adc_channel_t channel = ADC_CHANNEL_6;     //GPIO34 if ADC1, GPIO14 if ADC2
const adc_atten_t atten = ADC_ATTEN_DB_0;
const adc_unit_t unit = ADC_UNIT_1;

xQueueHandle timer_queue; // Initialize queue handler for timer-based events

bool encoder_is_white = false;
int encoder_count;
int steer_direction = 0; // 0: neutral | 1: right | 2: left
bool running = false;
bool master_key = false;
float speed = 0.0;

void mcpwm_example_gpio_initialize(void)
{
    printf("initializing mcpwm servo control gpio......\n");
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, 18);    //Set GPIO 18 as PWM0A, to which esc servo is connected
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM1A, 26);    //Set GPIO 19 as PWM1A, to which steering servo is connected
}

uint32_t servo_per_degree_init(uint32_t degree_of_rotation)
{
    uint32_t cal_pulsewidth = 0;
    cal_pulsewidth = (SERVO_MIN_PULSEWIDTH + (((SERVO_MAX_PULSEWIDTH - SERVO_MIN_PULSEWIDTH) * (degree_of_rotation)) / (SERVO_MAX_DEGREE)));
    return cal_pulsewidth;
}

uint32_t steering_servo_per_degree_init(uint32_t degree_of_rotation)
{
    uint32_t cal_pulsewidth = 0;
    cal_pulsewidth = (STEERING_SERVO_MIN_PULSEWIDTH + (((STEERING_SERVO_MAX_PULSEWIDTH - STEERING_SERVO_MIN_PULSEWIDTH) * (degree_of_rotation)) / (STEERING_SERVO_MAX_DEGREE)));
    return cal_pulsewidth;
}

void check_efuse(void)
{
    //Check TP is burned into eFuse
    if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_TP) == ESP_OK) {
        printf("eFuse Two Point: Supported\n");
    } else {
        printf("eFuse Two Point: NOT supported\n");
    }

    //Check Vref is burned into eFuse
    if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_VREF) == ESP_OK) {
        printf("eFuse Vref: Supported\n");
    } else {
        printf("eFuse Vref: NOT supported\n");
    }
}

void print_char_val_type(esp_adc_cal_value_t val_type)
{
    if (val_type == ESP_ADC_CAL_VAL_EFUSE_TP) {
        printf("Characterized using Two Point Value\n");
    } else if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
        printf("Characterized using eFuse Vref\n");
    } else {
        printf("Characterized using Default Vref\n");
    }
}

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

// Initialize timer 0 in group 0 for 1 sec alarm interval and auto reload
void alarm_init() {
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
}

// HCSR04 Ultrasonic Sensor Trigger initializations
void HCSR04_tx_init() {
  rmt_config_t rmt_tx;
  rmt_tx.channel = RMT_TX_CHANNEL;
  rmt_tx.gpio_num = RMT_TX_GPIO_NUM;
  rmt_tx.mem_block_num = 1;
  rmt_tx.clk_div = RMT_CLK_DIV;
  rmt_tx.tx_config.loop_en = false;
  rmt_tx.tx_config.carrier_duty_percent = 50;
  rmt_tx.tx_config.carrier_freq_hz = 3000;
  rmt_tx.tx_config.carrier_level = 1;
  rmt_tx.tx_config.carrier_en = RMT_TX_CARRIER_EN;
  rmt_tx.tx_config.idle_level = 0;
  rmt_tx.tx_config.idle_output_en = true;
  rmt_tx.rmt_mode = 0;
  rmt_config(&rmt_tx);
  rmt_driver_install(rmt_tx.channel, 0, 0);
}

// HCSR04 Ultrasonic Sensor Echo initializations
void HCSR04_rx_init() {
  rmt_config_t rmt_rx;
  rmt_rx.channel = RMT_RX_CHANNEL;
  rmt_rx.gpio_num = RMT_RX_GPIO_NUM;
  rmt_rx.clk_div = RMT_CLK_DIV;
  rmt_rx.mem_block_num = 1;
  rmt_rx.rmt_mode = RMT_MODE_RX;
  rmt_rx.rx_config.filter_en = true;
  rmt_rx.rx_config.filter_ticks_thresh = 100;
  rmt_rx.rx_config.idle_threshold = rmt_item32_tIMEOUT_US / 10 * (RMT_TICK_10_US);
  rmt_config(&rmt_rx);
  rmt_driver_install(rmt_rx.channel, 1000, 0);
}

// The main task of this example program
void timer_evt_task(void *arg) {
    while (1) {
        // Create dummy structure to store structure from queue
        timer_event_t evt;

        // Transfer from queue
        xQueueReceive(timer_queue, &evt, portMAX_DELAY);

        // Do something if triggered!
        if (evt.flag == 1) {
          // printf("triggered\n");
          // calc speed based on optical encoder count
          // Round Per Sec (round/s) *  0.62 (m/round) = speed (m/s)
          float round;
          if (encoder_count == 0) round = 0;
          else                    round = (float)(encoder_count / 8.0); // 8 optical or 12 optical pattern
          speed = round * 0.62;
          encoder_count = 0;
          // printf("Speed: %f\n", speed);
        }
    }
}

/**
 * @brief Configure MCPWM module
 */
void mcpwm_example_esc_servo_control(void *arg)
{
    uint32_t angle, count;
    //1. mcpwm gpio initialization
    // mcpwm_example_gpio_initialize();

    //2. initial mcpwm configuration
    printf("Configuring Initial Parameters of mcpwm......\n");
    mcpwm_config_t pwm_config;
    pwm_config.frequency = 50;    //frequency = 50Hz, i.e. for every servo motor time period should be 20ms
    pwm_config.cmpr_a = 0;    //duty cycle of PWMxA = 0
    pwm_config.cmpr_b = 0;    //duty cycle of PWMxb = 0
    pwm_config.counter_mode = MCPWM_UP_COUNTER;
    pwm_config.duty_mode = MCPWM_DUTY_MODE_0;
    mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &pwm_config);    //Configure PWM0A & PWM0B with above settings

    printf("Calibrating servo motor.......\n");
    vTaskDelay(3000 / portTICK_PERIOD_MS);  // Give yourself time to turn on crawler (3s)
    mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, 1200); // NEUTRAL signal in microseconds
    vTaskDelay(3100 / portTICK_PERIOD_MS); // Do for at least 3s, and leave in neutral state
    printf("Calibrating servo motor completed\n");
    
    count = 0;
    while (1) {
      if (master_key) {
        if (running) {
          if (count > 42) count = 42;
          else count++;
        } else {
          if (count < 3) count = 3;
          else count--;
        }
        angle = servo_per_degree_init(count);
        // printf("pulse width: %dus\n", angle);
        mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, angle);
        //Add delay, since it takes time for servo to rotate, generally 100ms/60degree rotation at 5V
        vTaskDelay(100 / portTICK_PERIOD_MS);
      } else {
        // printf("Send start signal...\n");
        if (count < 3) count = 3;
        else count--;
        angle = servo_per_degree_init(count);
        mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, angle);
        vTaskDelay(100 / portTICK_PERIOD_MS);
      }
  }
}

void mcpwm_example_steering_servo_control(void *arg)
{
    uint32_t angle, count;
    //1. mcpwm gpio initialization
    // mcpwm_example_gpio_initialize();

    //2. initial mcpwm configuration
    printf("Configuring Initial Parameters of mcpwm......\n");
    mcpwm_config_t pwm_config;
    pwm_config.frequency = 50;    //frequency = 50Hz, i.e. for every servo motor time period should be 20ms
    pwm_config.cmpr_a = 0;    //duty cycle of PWMxA = 0
    pwm_config.cmpr_b = 0;    //duty cycle of PWMxb = 0
    pwm_config.counter_mode = MCPWM_UP_COUNTER;
    pwm_config.duty_mode = MCPWM_DUTY_MODE_0;
    mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_1, &pwm_config);    //Configure PWM0A & PWM0B with above settings
    
    count = 0;
    while (1) {
      if (steer_direction == 1) {
        if (count < STEERING_SERVO_MAX_DEGREE) count++;
        else count = STEERING_SERVO_MAX_DEGREE;
      } else if (steer_direction == 2) {
        if (count > 0) count --;
        else count = 0;
      } else {
        if (count > 53) count--;
        else if (count < 53) count++;
      }
      angle = steering_servo_per_degree_init(count);
      mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_1, MCPWM_OPR_A, angle);
      //Add delay, since it takes time for servo to rotate, generally 100ms/60degree rotation at 5V
      vTaskDelay(100 / portTICK_PERIOD_MS);     
    }
}

void wheel_speed(void *arg) {
  //Configure ADC
  if (unit == ADC_UNIT_1) {
      adc1_config_width(ADC_WIDTH_BIT_12);
      adc1_config_channel_atten(channel, atten);
  } else {
      adc2_config_channel_atten((adc2_channel_t)channel, atten);
  }

  //Characterize ADC
  adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
  esp_adc_cal_value_t val_type = esp_adc_cal_characterize(unit, atten, ADC_WIDTH_BIT_12, DEFAULT_VREF, adc_chars);
  print_char_val_type(val_type);

  //Continuously sample ADC1
  while (1) {
      uint32_t adc_reading = 0;
      //Multisampling
      for (int i = 0; i < NO_OF_SAMPLES; i++) {
          if (unit == ADC_UNIT_1) {
              adc_reading += adc1_get_raw((adc1_channel_t)channel);
          } else {
              int raw;
              adc2_get_raw((adc2_channel_t)channel, ADC_WIDTH_BIT_12, &raw);
              adc_reading += raw;
          }
          vTaskDelay(pdMS_TO_TICKS(20/NO_OF_SAMPLES));
      }
      adc_reading /= NO_OF_SAMPLES;
      //Convert adc_reading to voltage in mV
      uint32_t voltage = esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);
      // printf("Raw: %d\tVoltage: %dmV\n", adc_reading, voltage);
      if (adc_reading < 1000 && encoder_is_white) {
        encoder_is_white = false; // now it's dark
        encoder_count++;
      } else if (adc_reading > 1000 && !encoder_is_white) {
        encoder_is_white = true;
        encoder_count++;
      }
      // vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

void hcsr04_task(void *arg) {
  printf("hello\n");
  HCSR04_tx_init();
  HCSR04_rx_init();
  rmt_item32_t item;
  item.level0 = 1;
  item.duration0 = RMT_TICK_10_US;
  item.level1 = 0;
  item.duration1 = RMT_TICK_10_US; // for one pulse this doesn't matter

  size_t rx_size = 0;
  RingbufHandle_t rb = NULL;
  rmt_get_ringbuf_handle(RMT_RX_CHANNEL, &rb);
  rmt_rx_start(RMT_RX_CHANNEL, 1);

  double distance = 0;

  while(1) {
    rmt_write_items(RMT_TX_CHANNEL, &item, 1, true);
    rmt_wait_tx_done(RMT_TX_CHANNEL, portMAX_DELAY);
    rmt_item32_t* item = (rmt_item32_t*)xRingbufferReceive(rb, &rx_size, 1000); // << here
    distance = 340.29 * ITEM_DURATION(item->duration0) / (1000 * 1000 * 2); // distance in meters
    distance *= 100;
    printf("Distance is %f cm\n", distance); // distance in centimeters
    if (distance < 35 && distance > 5) {
      running = false;
    } else {
      running = true;
    }

    vRingbufferReturnItem(rb, (void*) item);
    vTaskDelay(20 / portTICK_PERIOD_MS);
  }
}

void app_main(void)
{
    // pwm init
    mcpwm_example_gpio_initialize();

    // Check if Two Point or Vref are burned into eFuse
    check_efuse();

    // timer init
    timer_queue = xQueueCreate(10, sizeof(timer_event_t));
    alarm_init();

    // udp module initialization
    ESP_ERROR_CHECK( nvs_flash_init() );
    initialise_wifi();
    wait_for_ip();

    // initialize i2c
    i2c_example_master_init();
    i2c_scanner();

    printf("Testing servo motor.......\n");
    xTaskCreate(mcpwm_example_esc_servo_control, "mcpwm_example_esc_servo_control", 4096, NULL, 5, NULL);
    xTaskCreate(mcpwm_example_steering_servo_control, "mcpwm_example_steering_servo_control", 4096, NULL, 5, NULL);

    printf("Monitor wheel speed.......\n");
    xTaskCreate(wheel_speed, "wheel_speed", 4096, NULL, 5, NULL);

    // timer
    printf("Timer task start.......\n");
    xTaskCreate(timer_evt_task, "timer_evt_task", 4096, NULL, 5, NULL);

    // Ultrasonic sensor
    // printf("Ultrasonic range sensor.......\n");
    xTaskCreate(hcsr04_task, "hcsr04_task", 4096, NULL, 5, NULL);

    // udp
    xTaskCreate(udp_server_task, "udp_server", 4096, NULL, 5, NULL);

    // display
    xTaskCreate(display_state, "display_state", 4096, NULL, 5, NULL);
}