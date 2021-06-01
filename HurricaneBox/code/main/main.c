#include "main.h"
#include "udp.h"
#include "ledc.h"
#include "accelerometer.h"
#include "thermistor.h"

// Global vars
char led_intensity;
double accelerometer_values[3]; //x,y,z-axis
double roll;
double pitch;
double thermistor_value;

void app_main()
{
  // udp module initialization
  ESP_ERROR_CHECK( nvs_flash_init() );
  initialise_wifi();
  wait_for_ip();

  // ledc module initialization
  ledc_init();

  // accelerometer module initialization
  accelerometer_init();

  // thermistor module initialization 
  thermistor_init(); // i2c initialization
    

  // udp
  xTaskCreate(udp_client_task, "udp_client", 1024*2, NULL, configMAX_PRIORITIES, NULL);
  xTaskCreate(udp_server_task, "udp_server", 1024*2, NULL, configMAX_PRIORITIES, NULL);

  // ledc
  xTaskCreate(ledc_0, "ledc_0", 1024*2, NULL, configMAX_PRIORITIES, NULL);
  // xTaskCreate(ledc_1, "ledc_1", 1024*2, NULL, configMAX_PRIORITIES, NULL);

  // Create task to poll ADXL343
  xTaskCreate(test_adxl343, "test_adxl343", 1024*2, NULL, configMAX_PRIORITIES, NULL);

  // thermistor
  xTaskCreate(thermistor, "thermistor", 1024*2, NULL, configMAX_PRIORITIES, NULL);
}