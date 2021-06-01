/*
  Hayato Nakamura
  Yang Hang Liu
  Arnaud Harmange
*/

#include "main.h"
#include "udp.h"

// global vars
extern state_e state;	// Starting state
extern state_e next_state;
extern event_e event;
extern int minID;        // minID is equal to MYID at initial state (potential leader)
extern int election_timer;  // timer to keep track ELECTION_TIMEOUT_EVENT
extern int leader_timer;    // timer to keep track LEADER_TIMEOUT_EVENT
extern int heartbeat_timer; // timer to keep track HEARTBEAT_TIMEOUT_EVENT
extern char myColor;
extern bool voted;
extern bool vote_ready;
extern char voterID;

/* FreeRTOS event group to signal when we are connected & ready to make a request */
static EventGroupHandle_t wifi_event_group;

const int IPV4_GOTIP_BIT = BIT0;
const int IPV6_GOTIP_BIT = BIT1;

static const char *TAG = "example";
static char payload[100];
static char vote_msg[100];

int leader_id = 0;
int sock;
int sock_rx;
int addr_family;
int ip_protocol;



bool send_election_vote = false;

esp_err_t event_handler(void *ctx, system_event_t *event) {
    switch (event->event_id) {
    case SYSTEM_EVENT_STA_START:
        esp_wifi_connect();
        ESP_LOGI(TAG, "SYSTEM_EVENT_STA_START");
        break;
    case SYSTEM_EVENT_STA_CONNECTED:
        /* enable ipv6 */
        tcpip_adapter_create_ip6_linklocal(TCPIP_ADAPTER_IF_STA);
        break;
    case SYSTEM_EVENT_STA_GOT_IP:
        xEventGroupSetBits(wifi_event_group, IPV4_GOTIP_BIT);
        ESP_LOGI(TAG, "SYSTEM_EVENT_STA_GOT_IP");
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        /* This is a workaround as ESP32 WiFi libs don't currently auto-reassociate. */
        esp_wifi_connect();
        xEventGroupClearBits(wifi_event_group, IPV4_GOTIP_BIT);
        xEventGroupClearBits(wifi_event_group, IPV6_GOTIP_BIT);
        break;
    case SYSTEM_EVENT_AP_STA_GOT_IP6:
        xEventGroupSetBits(wifi_event_group, IPV6_GOTIP_BIT);
        ESP_LOGI(TAG, "SYSTEM_EVENT_STA_GOT_IP6");

        char *ip6 = ip6addr_ntoa(&event->event_info.got_ip6.ip6_info.ip);
        ESP_LOGI(TAG, "IPv6: %s", ip6);
    default:
        break;
    }
    return ESP_OK;
}

void initialise_wifi(void)
{
    tcpip_adapter_init();
    wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK( esp_event_loop_init(event_handler, NULL) );
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
        },
    };
    ESP_LOGI(TAG, "Setting WiFi configuration SSID %s...", wifi_config.sta.ssid);
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK( esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
    ESP_ERROR_CHECK( esp_wifi_start() );

    addr_family = AF_INET;
    ip_protocol = IPPROTO_IP;
    sock = socket(addr_family, SOCK_DGRAM, ip_protocol);
    if (sock < 0) {
        ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
    }
    ESP_LOGI(TAG, "Socket created");

    sock_rx = socket(addr_family, SOCK_DGRAM, ip_protocol);
    if (sock_rx < 0) {
        ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
    }
    ESP_LOGI(TAG, "Socket created");
}

void wait_for_ip()
{
    uint32_t bits = IPV4_GOTIP_BIT | IPV6_GOTIP_BIT ;

    ESP_LOGI(TAG, "Waiting for AP connection...");
    xEventGroupWaitBits(wifi_event_group, bits, false, true, portMAX_DELAY);
    ESP_LOGI(TAG, "Connected to AP");
}

void udp_client_task(void *pvParameters)
{
    char rx_buffer[128];
    char addr_str1[128];
    char addr_str2[128];
    char addr_str3[128];
    char addr_str4[128];
    char addr_str5[128];
    int addr_family;
    int ip_protocol;

    while (1) {
        struct sockaddr_in destAddr1;
        destAddr1.sin_addr.s_addr = inet_addr(ESPRESSIF_ID1_IP_ADDR);
        destAddr1.sin_family = AF_INET;
        destAddr1.sin_port = htons(ESPRESSIF_ID1_PORT);
        inet_ntoa_r(destAddr1.sin_addr, addr_str1, sizeof(addr_str1) - 1);

        struct sockaddr_in destAddr2;
        destAddr2.sin_addr.s_addr = inet_addr(ESPRESSIF_ID2_IP_ADDR);
        destAddr2.sin_family = AF_INET;
        destAddr2.sin_port = htons(ESPRESSIF_ID2_PORT);
        inet_ntoa_r(destAddr2.sin_addr, addr_str2, sizeof(addr_str2) - 1);

        struct sockaddr_in destAddr3;
        destAddr3.sin_addr.s_addr = inet_addr(ESPRESSIF_ID3_IP_ADDR);
        destAddr3.sin_family = AF_INET;
        destAddr3.sin_port = htons(ESPRESSIF_ID3_PORT);
        inet_ntoa_r(destAddr3.sin_addr, addr_str3, sizeof(addr_str3) - 1);

        struct sockaddr_in destAddr4;
        destAddr4.sin_addr.s_addr = inet_addr(ESPRESSIF_ID4_IP_ADDR);
        destAddr4.sin_family = AF_INET;
        destAddr4.sin_port = htons(ESPRESSIF_ID4_PORT);
        inet_ntoa_r(destAddr4.sin_addr, addr_str4, sizeof(addr_str4) - 1);

        struct sockaddr_in destAddr5;
        destAddr5.sin_addr.s_addr = inet_addr(ESPRESSIF_ID5_IP_ADDR);
        destAddr5.sin_family = AF_INET;
        destAddr5.sin_port = htons(ESPRESSIF_ID5_PORT);
        inet_ntoa_r(destAddr5.sin_addr, addr_str5, sizeof(addr_str5) - 1);

        int err;

        while (1) {
            if (event == ELECTION_TIMEOUT_EVENT) {
                printf("election timeout triggered\n");
                // message init
                // payload format "MYID, MINID, STATE"
                sprintf(payload, "%d,%d,%d", MYID, MYID, 2);

                // reset vote flag
                send_election_vote = false;

                // reset event
                event = NO_EVENT;
                if (state == ELECTION_STATE) {
                    // become leader
                    state = LEADER_STATE;
                    leader_id = MYID;
                    heartbeat_timer = 1;

                    // let others know that i am the new leader
                    if (MYID == 5) {
                        err = sendto(sock, payload, strlen(payload), 0, (struct sockaddr *)&destAddr1, sizeof(destAddr1));
                        err = sendto(sock, payload, strlen(payload), 0, (struct sockaddr *)&destAddr2, sizeof(destAddr2));
                        err = sendto(sock, payload, strlen(payload), 0, (struct sockaddr *)&destAddr3, sizeof(destAddr3));
                        err = sendto(sock, payload, strlen(payload), 0, (struct sockaddr *)&destAddr4, sizeof(destAddr4));
                    } else if (MYID == 4) {
                        err = sendto(sock, payload, strlen(payload), 0, (struct sockaddr *)&destAddr1, sizeof(destAddr1));
                        err = sendto(sock, payload, strlen(payload), 0, (struct sockaddr *)&destAddr2, sizeof(destAddr2));
                        err = sendto(sock, payload, strlen(payload), 0, (struct sockaddr *)&destAddr3, sizeof(destAddr3));
                        err = sendto(sock, payload, strlen(payload), 0, (struct sockaddr *)&destAddr5, sizeof(destAddr5));
                    } else if (MYID == 3) {
                        err = sendto(sock, payload, strlen(payload), 0, (struct sockaddr *)&destAddr1, sizeof(destAddr1));
                        err = sendto(sock, payload, strlen(payload), 0, (struct sockaddr *)&destAddr2, sizeof(destAddr2));
                        err = sendto(sock, payload, strlen(payload), 0, (struct sockaddr *)&destAddr4, sizeof(destAddr4));
                        err = sendto(sock, payload, strlen(payload), 0, (struct sockaddr *)&destAddr5, sizeof(destAddr5));
                    } else if (MYID == 2) {
                        // ESP_LOGI(TAG, "tag 2 send to tag 1");
                        err = sendto(sock, payload, strlen(payload), 0, (struct sockaddr *)&destAddr1, sizeof(destAddr1));
                        err = sendto(sock, payload, strlen(payload), 0, (struct sockaddr *)&destAddr3, sizeof(destAddr3));
                        err = sendto(sock, payload, strlen(payload), 0, (struct sockaddr *)&destAddr4, sizeof(destAddr4));
                        err = sendto(sock, payload, strlen(payload), 0, (struct sockaddr *)&destAddr5, sizeof(destAddr5));
                    } else if (MYID == 1) {
                        err = sendto(sock, payload, strlen(payload), 0, (struct sockaddr *)&destAddr2, sizeof(destAddr2));
                        err = sendto(sock, payload, strlen(payload), 0, (struct sockaddr *)&destAddr3, sizeof(destAddr3));
                        err = sendto(sock, payload, strlen(payload), 0, (struct sockaddr *)&destAddr4, sizeof(destAddr4));
                        err = sendto(sock, payload, strlen(payload), 0, (struct sockaddr *)&destAddr5, sizeof(destAddr5));
                    } else {
                        printf("Unknown ID\n");
                    }

                    // if (err < 0) {
                    //     ESP_LOGE(TAG, "Error occured during sending: errno %d", errno);
                    //     break;
                    // }
                    // ESP_LOGI(TAG, "Message sent");
                } else {
                    // state = NON_LEADER_STATE;
                    heartbeat_timer = 1;
                }
                // vTaskDelay(1000 / portTICK_PERIOD_MS);
            } else if (event == LEADER_TIMEOUT_EVENT) {
                vote_ready = false;
                printf("leader timeout triggered\n");
                // state = ELECTION_STATE;
                // message init
                // payload format "MYID, MINID, STATE"
                sprintf(payload, "%d,%d,%d", MYID, MYID, 0);
                send_election_vote = true;
                state = ELECTION_STATE;

                // reset event
                event = NO_EVENT;

                // start election
                if (MYID == 5) {
                    err = sendto(sock, payload, strlen(payload), 0, (struct sockaddr *)&destAddr1, sizeof(destAddr1));
                    err = sendto(sock, payload, strlen(payload), 0, (struct sockaddr *)&destAddr2, sizeof(destAddr2));
                    err = sendto(sock, payload, strlen(payload), 0, (struct sockaddr *)&destAddr3, sizeof(destAddr3));
                    err = sendto(sock, payload, strlen(payload), 0, (struct sockaddr *)&destAddr4, sizeof(destAddr4));
                } else if (MYID == 4) {
                    err = sendto(sock, payload, strlen(payload), 0, (struct sockaddr *)&destAddr1, sizeof(destAddr1));
                    err = sendto(sock, payload, strlen(payload), 0, (struct sockaddr *)&destAddr2, sizeof(destAddr2));
                    err = sendto(sock, payload, strlen(payload), 0, (struct sockaddr *)&destAddr3, sizeof(destAddr3));
                    err = sendto(sock, payload, strlen(payload), 0, (struct sockaddr *)&destAddr5, sizeof(destAddr5));
                } else if (MYID == 3) {
                    err = sendto(sock, payload, strlen(payload), 0, (struct sockaddr *)&destAddr1, sizeof(destAddr1));
                    err = sendto(sock, payload, strlen(payload), 0, (struct sockaddr *)&destAddr2, sizeof(destAddr2));
                    err = sendto(sock, payload, strlen(payload), 0, (struct sockaddr *)&destAddr4, sizeof(destAddr4));
                    err = sendto(sock, payload, strlen(payload), 0, (struct sockaddr *)&destAddr5, sizeof(destAddr5));
                } else if (MYID == 2) {
                    err = sendto(sock, payload, strlen(payload), 0, (struct sockaddr *)&destAddr1, sizeof(destAddr1));
                    err = sendto(sock, payload, strlen(payload), 0, (struct sockaddr *)&destAddr3, sizeof(destAddr3));
                    err = sendto(sock, payload, strlen(payload), 0, (struct sockaddr *)&destAddr4, sizeof(destAddr4));
                    err = sendto(sock, payload, strlen(payload), 0, (struct sockaddr *)&destAddr5, sizeof(destAddr5));
                } else if (MYID == 1) {
                    err = sendto(sock, payload, strlen(payload), 0, (struct sockaddr *)&destAddr2, sizeof(destAddr2));
                    err = sendto(sock, payload, strlen(payload), 0, (struct sockaddr *)&destAddr3, sizeof(destAddr3));
                    err = sendto(sock, payload, strlen(payload), 0, (struct sockaddr *)&destAddr4, sizeof(destAddr4));
                    err = sendto(sock, payload, strlen(payload), 0, (struct sockaddr *)&destAddr5, sizeof(destAddr5));
                } else {
                    printf("Unknown ID\n");
                }
            } else if (event == HEARTBEAT_TIMEOUT_EVENT) {
                printf("heartbeat timeout triggered\n");
                // reset event
                event = NO_EVENT;
                // reset timer
                heartbeat_timer = 1;
                if (state == LEADER_STATE) {
                    // send alive msg to members
                    // message init
                    // payload format "MYID, MINID, STATE"
                    sprintf(payload, "%d,%d,%d", MYID, MYID, 2);

                    // let others know that the leader is alive
                    if (MYID == 5) {
                        err = sendto(sock, payload, strlen(payload), 0, (struct sockaddr *)&destAddr1, sizeof(destAddr1));
                        err = sendto(sock, payload, strlen(payload), 0, (struct sockaddr *)&destAddr2, sizeof(destAddr2));
                        err = sendto(sock, payload, strlen(payload), 0, (struct sockaddr *)&destAddr3, sizeof(destAddr3));
                        err = sendto(sock, payload, strlen(payload), 0, (struct sockaddr *)&destAddr4, sizeof(destAddr4));
                    } else if (MYID == 4) {
                        err = sendto(sock, payload, strlen(payload), 0, (struct sockaddr *)&destAddr1, sizeof(destAddr1));
                        err = sendto(sock, payload, strlen(payload), 0, (struct sockaddr *)&destAddr2, sizeof(destAddr2));
                        err = sendto(sock, payload, strlen(payload), 0, (struct sockaddr *)&destAddr3, sizeof(destAddr3));
                        err = sendto(sock, payload, strlen(payload), 0, (struct sockaddr *)&destAddr5, sizeof(destAddr5));
                    } else if (MYID == 3) {
                        err = sendto(sock, payload, strlen(payload), 0, (struct sockaddr *)&destAddr1, sizeof(destAddr1));
                        err = sendto(sock, payload, strlen(payload), 0, (struct sockaddr *)&destAddr2, sizeof(destAddr2));
                        err = sendto(sock, payload, strlen(payload), 0, (struct sockaddr *)&destAddr4, sizeof(destAddr4));
                        err = sendto(sock, payload, strlen(payload), 0, (struct sockaddr *)&destAddr5, sizeof(destAddr5));
                    } else if (MYID == 2) {
                        err = sendto(sock, payload, strlen(payload), 0, (struct sockaddr *)&destAddr1, sizeof(destAddr1));
                        err = sendto(sock, payload, strlen(payload), 0, (struct sockaddr *)&destAddr3, sizeof(destAddr3));
                        err = sendto(sock, payload, strlen(payload), 0, (struct sockaddr *)&destAddr4, sizeof(destAddr4));
                        err = sendto(sock, payload, strlen(payload), 0, (struct sockaddr *)&destAddr5, sizeof(destAddr5));
                    } else if (MYID == 1) {
                        err = sendto(sock, payload, strlen(payload), 0, (struct sockaddr *)&destAddr2, sizeof(destAddr2));
                        err = sendto(sock, payload, strlen(payload), 0, (struct sockaddr *)&destAddr3, sizeof(destAddr3));
                        err = sendto(sock, payload, strlen(payload), 0, (struct sockaddr *)&destAddr4, sizeof(destAddr4));
                        err = sendto(sock, payload, strlen(payload), 0, (struct sockaddr *)&destAddr5, sizeof(destAddr5));
                    } else {
                        printf("Unknown ID\n");
                    }

                    // reset leader timer
                    leader_timer = 1;
                }
            }

            if (state == NON_LEADER_STATE || state == LEADER_STATE) {
                if (vote_ready) {
                    // send voting info to leader
                    printf("***************\n");
                    printf("RGB VOTE TO LEADER\n");
                    printf("***************\n");
                    sprintf(payload, "%c,%c,%d", myColor, voterID, 3); // 3 means voting info
                    if (leader_id == 1) {
                        sendto(sock, payload, strlen(payload), 0, (struct sockaddr *)&destAddr1, sizeof(destAddr1));
                        printf("sent to leader 1\n");
                    } else if (leader_id == 2) {
                        sendto(sock, payload, strlen(payload), 0, (struct sockaddr *)&destAddr2, sizeof(destAddr2));
                        printf("sent to leader 2\n");
                    } else if (leader_id == 3) {
                        sendto(sock, payload, strlen(payload), 0, (struct sockaddr *)&destAddr3, sizeof(destAddr3));
                        printf("sent to leader 3\n");
                    } else if (leader_id == 4) {
                        sendto(sock, payload, strlen(payload), 0, (struct sockaddr *)&destAddr4, sizeof(destAddr4));
                        printf("sent to leader 4\n");
                    } else if (leader_id == 5) {
                        sendto(sock, payload, strlen(payload), 0, (struct sockaddr *)&destAddr5, sizeof(destAddr5));
                        printf("sent to leader 5\n");
                    }
                    vote_ready = false;
                }
            }
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }


        if (sock != -1) {
            ESP_LOGE(TAG, "Shutting down socket and restarting...");
            shutdown(sock, 0);
            close(sock);
        }
    }
    vTaskDelete(NULL);
}

// receive intensity value from web client, store in rx_buffer
void udp_server_task(void *pvParameters)
{
    char rx_buffer[128];
    char addr_str[128];
    int addr_family;
    int ip_protocol;

    while (1) {
        struct sockaddr_in destAddr;
        destAddr.sin_addr.s_addr = htonl(INADDR_ANY);
        destAddr.sin_family = AF_INET;
        destAddr.sin_port = htons(LISTEN_PORT);
        addr_family = AF_INET;
        ip_protocol = IPPROTO_IP;
        inet_ntoa_r(destAddr.sin_addr, addr_str, sizeof(addr_str) - 1);

        int err = bind(sock_rx, (struct sockaddr *)&destAddr, sizeof(destAddr));
        if (err < 0) {
            ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
        }
        ESP_LOGI(TAG, "Socket binded");

        while (1) {

            ESP_LOGI(TAG, "Waiting for data");
            struct sockaddr_in6 sourceAddr; // Large enough for both IPv4 or IPv6
            socklen_t socklen = sizeof(sourceAddr);
            ESP_LOGI(TAG, "hahahahahah");
            int len = recvfrom(sock_rx, rx_buffer, sizeof(rx_buffer) - 1, 0, (struct sockaddr *)&sourceAddr, &socklen);
            ESP_LOGI(TAG, "oh?");

            // Error occured during receiving
            if (len < 0) {
                ESP_LOGE(TAG, "recvfrom failed: errno %d", errno);
                break;
            }
            // Data received
            else {
                // Get the sender's ip address as string
                if (sourceAddr.sin6_family == PF_INET) {
                    inet_ntoa_r(((struct sockaddr_in *)&sourceAddr)->sin_addr.s_addr, addr_str, sizeof(addr_str) - 1);
                } else if (sourceAddr.sin6_family == PF_INET6) {
                    inet6_ntoa_r(sourceAddr.sin6_addr, addr_str, sizeof(addr_str) - 1);
                }

                rx_buffer[len] = 0; // Null-terminate whatever we received and treat like a string...
                ESP_LOGI(TAG, "Received %d bytes from %s:", len, addr_str);
                ESP_LOGI(TAG, "Received ID %c", rx_buffer[0]);
                ESP_LOGI(TAG, "MinID %c", rx_buffer[2]);
                ESP_LOGI(TAG, "%s", rx_buffer);
                // led_intensity = rx_buffer[0];
                if (rx_buffer[4] == '2') { // from leader
                    leader_timer = 1;
                    ESP_LOGI(TAG, "%d", leader_timer);
                    state = NON_LEADER_STATE;
                    send_election_vote = false;
                    leader_id = (int)(rx_buffer[0] - '0');
                    // vote_ready = true;
                } else if (rx_buffer[4] == '0') {
                    // other fob started leader election
                    state = ELECTION_STATE;
                    if (send_election_vote == false) {
                        // trigger election
                        leader_timer = LEADER_TIMEOUT;
                    }

                    if ((int)rx_buffer[0] < (int)('0'+MYID)) {
                        // I'm not qualified to be a aleader
                        state = NON_LEADER_STATE;
                    } 

                } else if (rx_buffer[4] == '3') { // get voting from peers
                    sprintf(vote_msg, "%c,%c", rx_buffer[0], rx_buffer[2]);
                    udp_client_dataserver();
                    printf("***************\n");
                    printf("RGB VOTE TO SERVER\n");
                    printf("***************\n");
                }
            }
        }

        if (sock_rx != -1) {
            ESP_LOGE(TAG, "Shutting down socket and restarting...");
            shutdown(sock_rx, 0);
            close(sock_rx);
        }
    }
    vTaskDelete(NULL);
}

void udp_client_dataserver() {
    char rx_buffer[128];
    char addr_str[128];
    int addr_family;
    int ip_protocol;

    struct sockaddr_in destAddr;
    destAddr.sin_addr.s_addr = inet_addr(SERVER_ADDR);
    destAddr.sin_family = AF_INET;
    destAddr.sin_port = htons(SERVER_PORT);
    inet_ntoa_r(destAddr.sin_addr, addr_str, sizeof(addr_str) - 1);

    int err;
    err = sendto(sock_rx, vote_msg, strlen(vote_msg), 0, (struct sockaddr *)&destAddr, sizeof(destAddr));
    if (err < 0) {
        ESP_LOGE(TAG, "Error occured during sending: errno %d", errno);
    }
    ESP_LOGI(TAG, "Message sent to the server");
}