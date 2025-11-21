#ifndef _esp_nvs_H_
#define _esp_nvs_H_

#include <stdio.h>
#include "esp_err.h"
#include "esp_check.h"

typedef struct esp_nvs_t *esp_nvs_handle_t;

typedef struct{
    char name_space[64];
    char key[64];
    char *value;
    size_t value_size;
} esp_nvs_config_t;

esp_err_t init_esp_nvs(esp_nvs_config_t *config, esp_nvs_handle_t *handle);

void plot_nvs_stats(void);

void esp_nvs_write_string(const char *value, esp_nvs_handle_t esp_nvs_handle);

esp_err_t esp_nvs_read_string(esp_nvs_handle_t esp_nvs_handle, char **out_str);

void esp_nvs_delete_string(esp_nvs_handle_t esp_nvs_handle);

void esp_nvs_deinit(esp_nvs_handle_t esp_nvs_handle);

void esp_nvs_change_key(const char *new_key, esp_nvs_handle_t esp_nvs_handle);

void esp_nvs_list_namespaces(void);

#endif