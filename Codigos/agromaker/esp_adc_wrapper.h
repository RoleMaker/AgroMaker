#ifndef _esp_adc_wrapper_H_
#define _esp_adc_wrapper_H_

#include "esp_err.h"
#include "esp_check.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


typedef struct esp_adc_wrapper_t *esp_adc_wrapper_handle_t;

typedef struct{
    adc_channel_t adc_channel;  // ADC channel to read from
    adc_oneshot_unit_handle_t adc_handle;
       
} esp_adc_wrapper_config_t;


esp_err_t init_esp_adc_wrapper(esp_adc_wrapper_config_t *config, esp_adc_wrapper_handle_t *handle);

void read_esp_adc_wrapper(esp_adc_wrapper_handle_t handle, int *raw_value, int *voltage_value);

adc_oneshot_unit_handle_t get_adc_handle(esp_adc_wrapper_handle_t handle);

void read_esp_adc_wrapper_n_times(esp_adc_wrapper_handle_t handle, int *raw_value, int *voltage_value, int n_times);

  
#endif