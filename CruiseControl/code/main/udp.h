/*
  Hayato Nakamura
  Yang Hang Liu
  Arnaud Harmange
*/

#ifndef UDP_H
#define UDP_H 

#include "main.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>

#define WIFI_SSID "Monster Hunters"
#define WIFI_PASS "WeAreTheBestHunters466"

#define PORT 1131

esp_err_t event_handler(void *ctx, system_event_t *event);

void initialise_wifi(void);

void wait_for_ip();

// send accelerometer and temp values to web client
void udp_client_task(void *pvParameters);

// receive intensity value from web client, store in rx_buffer
void udp_server_task(void *pvParameters);

#endif