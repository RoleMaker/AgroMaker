#include <stdio.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/adc.h"

#define ADC_CHANNEL ADC1_CHANNEL_0 // GPIO36

void app_main(void) {
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC_CHANNEL, ADC_ATTEN_DB_11);

    while (1) {
        int raw = adc1_get_raw(ADC_CHANNEL);
        double voltage = raw  (3.3 / 4095.0);
        double resistance = (3.3 - voltage)  10000 / voltage;
        double temperature = 1 / (log(resistance / 10000) / 3950 + 1 / 298.15) - 273.15;
        printf("Temperature: %.2f °C\n", temperature);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

  

  

