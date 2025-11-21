#include <stdio.h>
#include "esp_coiiote.h"
#include "esp_wifi_interface.h"
#include "esp_mqtt_interface.h"
#include "esp_adc_wrapper.h"
#include "dht.h"
#include "esp_log.h"
#include <math.h>

#define LED_STATUS 2
#define LED_RESET 0
#define HOST "coiiote.com" 

static void evento_mqtt(uint32_t received_id, const char *topic, const char *data)
{

    switch ((esp_mqtt_event_id_t)received_id)
    {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI("MQTT_EVENT", "Conectado ao broker MQTT");

        char topic_to_subscribe[100];
        snprintf(topic_to_subscribe, sizeof(topic_to_subscribe), "%s/%s/ota",
                 (char *)esp_coiiote_get_workspace(),
                 (char *)esp_coiiote_get_thingname());       // Construct the topic using MAC address, thing name, and workspace
        esp_mqtt_interface_subscribe(topic_to_subscribe, 0); // Inscreve-se no tópico para receber mensagens

        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI("MQTT_EVENT", "Desconectado do broker MQTT");
        break;
    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI("MQTT_EVENT", "Inscrito no tópico: %s", topic);
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI("MQTT_EVENT", "Desinscrito do tópico: %s", topic);
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI("MQTT_EVENT", "Publicado no tópico: %s", topic);
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGI("MQTT_EVENT", "Dados recebidos no tópico: %s, Dados: %s", topic, data);

        char otatopic[100];
        snprintf(otatopic, sizeof(otatopic), "%s/%s/ota",
                 (char *)esp_coiiote_get_workspace(),
                 (char *)esp_coiiote_get_thingname()); // Construct the topic using MAC address, thing name, and workspace
        if (strcmp(topic, otatopic) == 0)
        {
            char otadata[100];
            snprintf(otadata, sizeof(otadata), "http://coiiote.com/ota/%s",
                     data); // Construct the topic using MAC address, thing name, and workspace

            // função esp_coiiote para atulizar firmware via OTA
            esp_coiiote_ota(otadata); // Call the function to update firmware via OTA
        }
        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGE("MQTT_EVENT", "Erro no evento MQTT");
        break;
    default:
        ESP_LOGI("MQTT_EVENT", "Outro evento: %lu", received_id);
    }
}

static void http_test_task(void *pvParameters)
{
    
    
    // http_rest_with_hostname_path(); // HTTP REST API example
    esp_coiiote_access(); // Send data to CoIIoTe server

    esp_coiiote_config(); // send configuration to CoIIoTe server

    vTaskDelete(NULL); // Delete the task after completion
}

void app_main(void)
{

    esp_wifi_interface_config_t wifi_inteface_config = {
        .channel = 1,                                               // Access point channel
        .esp_max_retry = 5,                                         // Maximum number of retries to connect to the AP
        .wifi_sae_mode = WPA3_SAE_PWE_BOTH,                         // SAE mode for WPA3
        .esp_wifi_scan_auth_mode_treshold = WIFI_AUTH_WPA_WPA2_PSK, // Authentication mode threshold for Wi-Fi scan
        .status_io = LED_STATUS,                                    // Connection status.
        .reset_io = LED_RESET,                                      // Reset pin.
    };
    WiFiInit(&wifi_inteface_config);
    WiFiSimpleConnection();

    esp_coiiote_config_t coiiote_config = {
        .server = HOST,
        .port = 443,
    };
    esp_coiiote_init(&coiiote_config);

    //save ip
    esp_local_ip(WiFiGetLocalIP());

    esp_mqtt_interface_config_t client_mqtt = {
        .host = HOST,
        .port = 1883,
        .username = esp_coiiote_get_mac_str(),
        .password = (char *)esp_coiiote_get_thing_password(),
        .id = esp_coiiote_get_mac_str(),
    };
    esp_mqtt_interface_init(&client_mqtt);
    esp_mqtt_interface_register_cb(evento_mqtt);
    esp_coiiote_debug();

    xTaskCreate(&http_test_task, "http_test_task", 10000, NULL, 5, NULL);

    esp_coiiote_webserver_init();

    esp_adc_wrapper_config_t config = {
        .adc_channel = 0,          // ADC channel to read from
        .adc_handle = NULL,        // ADC handle for ADC1;
    };
    esp_adc_wrapper_handle_t luminosidade_handle;
    esp_err_t ret = init_esp_adc_wrapper(&config, &luminosidade_handle);
    
    config.adc_channel = 3; //
    config.adc_handle = get_adc_handle(luminosidade_handle);
    esp_adc_wrapper_handle_t temp_handle;
    ret = init_esp_adc_wrapper(&config, &temp_handle);
 
    config.adc_channel = 6; //
    esp_adc_wrapper_handle_t voltage_handle;
    ret = init_esp_adc_wrapper(&config, &voltage_handle);

    config.adc_channel = 7; //
    esp_adc_wrapper_handle_t solo_handle;
    ret = init_esp_adc_wrapper(&config, &solo_handle);
 

    int raw_value = 0; // Variable to store the raw ADC value
    int voltage_value = 0; // Variable to store the voltage value

    int n_times = 100; // Number of readings to average

    
    while (1)
    {

        vTaskDelay(20000 / portTICK_PERIOD_MS); // Wait for 20 seconds before sending data
        
        // Send voltage data
        char topic[100];
        snprintf(topic, sizeof(topic), "%s/%s/voltage",
                 (char *)esp_coiiote_get_workspace(),
                 (char *)esp_coiiote_get_thingname()); // Construct the topic using MAC address, thing name, and workspace
        read_esp_adc_wrapper_n_times(voltage_handle, &raw_value, &voltage_value, n_times);
        char data[50];
        voltage_value = (voltage_value * 2); // voltage divider of 1/2
        snprintf(data, sizeof(data), "%d", voltage_value);
        esp_mqtt_interface_publish(topic, data, 0, 0);
        uint32_t ADC_max = 3200; // Maximum ADC value (3.3
        
        // Send luminosity data
        snprintf(topic, sizeof(topic), "%s/%s/lum",
                 (char *)esp_coiiote_get_workspace(),
                 (char *)esp_coiiote_get_thingname()); // Construct the topic using MAC address, thing name, and workspace
        read_esp_adc_wrapper_n_times(luminosidade_handle, &raw_value, &voltage_value, n_times);
        uint32_t ADC_min = 140;    // Minimum ADC value (0V)
        voltage_value = ADC_max - voltage_value; // inverte o valor para o sensor de luminosidade
        if (voltage_value < ADC_min) voltage_value = ADC_min;   
        float PerLumin = (float)((float)voltage_value / ((float)ADC_max - (float)ADC_min)) * 100.0;
        snprintf(data, sizeof(data), "%.1f", PerLumin);
        esp_mqtt_interface_publish(topic, data, 0, 0);
        
        // Send temperature data
        snprintf(topic, sizeof(topic), "%s/%s/ttemp",
                 (char *)esp_coiiote_get_workspace(),
                 (char *)esp_coiiote_get_thingname()); // Construct the topic using MAC address, thing name, and workspace
        read_esp_adc_wrapper_n_times(temp_handle, &raw_value, &voltage_value, n_times);
        uint32_t max_voltage = ADC_max; // in millivolts
        uint32_t R1 = 10000;    // Resistance of R1 in ohms
        float R2 = ((float)max_voltage / (float)voltage_value - 1) * (float)R1; // Calculate R2 based on voltage divider formula
        float logR2 = log(R2);
        float T = (1.0 / (0.001129148 + (0.000234125 * logR2) + (0.0000000876741 * logR2 * logR2 * logR2))); // Calculate temperature in Kelvin using the Steinhart-Hart equation
        T = T - 273.15; // Convert Kelvin to Celsius
        snprintf(data, sizeof(data), "%.1f", T);
        esp_mqtt_interface_publish(topic, data, 0, 0);
        
        // Send soil moisture data  
        snprintf(topic, sizeof(topic), "%s/%s/soil",
                 (char *)esp_coiiote_get_workspace(),
                 (char *)esp_coiiote_get_thingname()); // Construct the topic using MAC address, thing name, and workspace
        read_esp_adc_wrapper_n_times(solo_handle, &raw_value, &voltage_value, n_times);
        voltage_value = voltage_value-ADC_min; // inverte o valor para o sensor de luminosidade
        if (voltage_value < ADC_min) voltage_value = ADC_min;   
        float PerSoil = (float)((float)voltage_value / ((float)1500.0 - (float)ADC_min)) * 100.0;
        snprintf(data, sizeof(data), "%.1f", PerSoil);
        esp_mqtt_interface_publish(topic, data, 0, 0);
        
        // humidity and temperature data from DHT11
        float humidity = 0.0;
        float temperature = 0.0;
        if (dht_read_float_data(DHT_TYPE_DHT11, 32, &humidity, &temperature) == ESP_OK) {
            snprintf(topic, sizeof(topic), "%s/%s/dhthum",
                 (char *)esp_coiiote_get_workspace(),
                 (char *)esp_coiiote_get_thingname()); // Construct the topic using MAC address, thing name, and workspace
        
            snprintf(data, sizeof(data), "%.1f", humidity);
            esp_mqtt_interface_publish(topic, data, 0, 0);
            snprintf(topic, sizeof(topic), "%s/%s/dhttemp",
                 (char *)esp_coiiote_get_workspace(),
                 (char *)esp_coiiote_get_thingname()); // Construct the topic using MAC address, thing name, and workspace
            snprintf(data, sizeof(data), "%.1f", temperature);
            esp_mqtt_interface_publish(topic, data, 0, 0);

        } else {
            snprintf(data, sizeof(data), "Failed to read from DHT sensor");
        }
 

        vTaskDelay(600000 / portTICK_PERIOD_MS);
        esp_wifi_check_reset_button();

    }
}