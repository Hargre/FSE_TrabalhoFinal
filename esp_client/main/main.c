#include <stdio.h>
#include "nvs.h"
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "esp_system.h"
#include "freertos/semphr.h"

#include "driver/gpio.h"

#include "mqtt.h"
#include "wifi.h"
#include "led.h"
#include "climate.h"
#include "control.h"
#include "button.h"
#include "storage.h"

xSemaphoreHandle wifiSemaphore;
xSemaphoreHandle conexaoMQTTSemaphore;
xSemaphoreHandle transmitDataSemaphore;

Weather_t weather_data;
int storage_status;
char room[15];

void readsens(void *params) {
    while (1) {
        read_sensor(&weather_data);
        printf("t: %d\n", weather_data.temperature);
        printf("h: %d\n", weather_data.humidity);
        vTaskDelay(30000 / portTICK_PERIOD_MS);
    }
}

void conectadoWifi(void * params) {
    while(true) {
        if(xSemaphoreTake(wifiSemaphore, portMAX_DELAY)) {
            mqtt_start();
            if (!storage_status) {
                uint8_t mac[6] = {0};
                char topic[50];
                esp_efuse_mac_get_default(mac);
                sprintf(topic, "fse2020/150009313/dispositivos/%x%x%x%x%x%x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
                mqtt_send_message(topic, "{\"action\": \"new\"}");
                mqtt_subscribe(topic);
            }
        }
    }
}

void trataComunicacaoComServidor(void * params) {
    char tdata[20];
    char hdata[20];
    char ttopic[50];
    char htopic[50];
    while(true) {
        if  (xSemaphoreTake(conexaoMQTTSemaphore, portMAX_DELAY) && xSemaphoreTake(transmitDataSemaphore, portMAX_DELAY)) {
            sprintf(tdata, "{\"t\": \"%d\"}", weather_data.temperature);
            sprintf(ttopic, "fse2020/150009313/%s/temperatura", room);
            mqtt_send_message(ttopic, tdata);
            sprintf(hdata, "{\"h\": \"%d\"}", weather_data.humidity);
            sprintf(htopic, "fse2020/150009313/%s/umidade", room);
            mqtt_send_message(htopic, hdata);

            xSemaphoreGive(conexaoMQTTSemaphore);
            xSemaphoreGive(transmitDataSemaphore);

            vTaskDelay(30000 / portTICK_PERIOD_MS);
        }
    }
}

void dispatchButtonState(void *params) {
    while (true) {
        if (xSemaphoreTake(conexaoMQTTSemaphore, portMAX_DELAY) &&
                xSemaphoreTake(transmitDataSemaphore, portMAX_DELAY)) {
            char topic[50];
            char data[20];
            sprintf(topic, "fse2020/150009313/%s/estado", room);
            sprintf(data, "{\"s\": \"%d\"}", (int)params);
            mqtt_send_message(topic, data);
            xSemaphoreGive(conexaoMQTTSemaphore);
            xSemaphoreGive(transmitDataSemaphore);
        }
    }
}

void app_main(void) {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // storage_status = get_initialization_status(room);
    // if (storage_status) {
    //     ESP_LOGI("Storage", "Room: %s", room);
    // } else {
    //     ESP_LOGI("Storage", "No room stored");
    // }

    wifiSemaphore = xSemaphoreCreateBinary();
    conexaoMQTTSemaphore = xSemaphoreCreateBinary();
    transmitDataSemaphore = xSemaphoreCreateBinary();

    init_led();
    wifi_start();
    xTaskCreate(&conectadoWifi, "Conecta MQTT", 4096, NULL, 1, NULL);
    init_sensor();
    xTaskCreate(&readsens, "Leitura dht", 2048, NULL, 1, NULL);
    setupButtonHandler();
    init_button();
    xTaskCreate(&trataComunicacaoComServidor, "Envia sensor", 4096, NULL, 1, NULL);
}
