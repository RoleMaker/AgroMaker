#ifndef _esp_mqtt_interface_H_
#define _esp_mqtt_interface_H_

#include "mqtt_client.h"
#include "freertos/queue.h"

typedef void (*esp_mqtt_interface_cb_t)(uint32_t event_id, const char *topic, const char *data);

typedef struct esp_mqtt_interface_t *esp_mqtt_interface_handle_t;

typedef struct {
    char *topic;  
    char *data;   
} mqtt_received_message_t;

typedef struct{
    char *uri;
    char *host;
    int port;
    char *username;
    char *password;
    char *id;
    
} esp_mqtt_interface_config_t;

void esp_mqtt_interface_init(esp_mqtt_interface_config_t *esp_mqtt_interface_config);

void esp_mqtt_interface_register_cb(esp_mqtt_interface_cb_t cb);

void mqtt_protocol_change_uri (esp_mqtt_client_handle_t client, const char *uri);

void esp_mqtt_interface_publish(const char *topic, const char *data, int qos, int retain);

void esp_mqtt_interface_subscribe(const char *topic, int qos);

#endif