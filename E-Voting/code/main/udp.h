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

// webserver/database address
#define SERVER_ADDR "10.0.0.30"
#define SERVER_PORT 1131
// list of other device local IPs
#define ESPRESSIF_ID1_IP_ADDR "10.0.0.85"
#define ESPRESSIF_ID2_IP_ADDR "10.0.0.123"
#define ESPRESSIF_ID3_IP_ADDR "10.0.0.145"
#define ESPRESSIF_ID4_IP_ADDR "10.0.0.47"
#define ESPRESSIF_ID5_IP_ADDR "10.0.0.204"
#define ESPRESSIF_ID1_PORT 1131
#define ESPRESSIF_ID2_PORT 1131
#define ESPRESSIF_ID3_PORT 1131
#define ESPRESSIF_ID4_PORT 1131
#define ESPRESSIF_ID5_PORT 1131

#define LISTEN_PORT 1131

#define MAXIMUM_FOBS 10
#define MSG_LENGTH 5

esp_err_t event_handler(void *ctx, system_event_t *event);

void initialise_wifi(void);

void wait_for_ip();

// send accelerometer and temp values to web client
void udp_client_task(void *pvParameters);

// receive intensity value from web client, store in rx_buffer
void udp_server_task(void *pvParameters);

void udp_client_dataserver();

#endif