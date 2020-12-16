#include <stdio.h>

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "freertos/semphr.h"

#include "cJSON.h"


#include "control.h"
#include "button.h"
#include "led.h"
#include "mqtt.h"
#include "storage.h"
#include "climate.h"

xQueueHandle interruptQueue;
extern char room[15];
extern int storage_status;
extern Weather_t weather_data;
extern xSemaphoreHandle transmitDataSemaphore;
extern xSemaphoreHandle conexaoMQTTSemaphore;
extern xSemaphoreHandle wifiSemaphore;


void IRAM_ATTR gpio_isr_handler(void *args) {
    int pin = (int)args;
    xQueueSendFromISR(interruptQueue, &pin, NULL);
}

void setupButtonHandler() {
    interruptQueue = xQueueCreate(10, sizeof(int));
    xTaskCreate(buttonHandler, "ButtonHandler", 2048, NULL, 1, NULL);
}

void buttonHandler(void *params) {
    int pin;
    while (true) {
        if (xQueueReceive(interruptQueue, &pin, portMAX_DELAY)) {
            int state = gpio_get_level(pin);
            printf("state: %d\n", state);
            gpio_isr_handler_remove(pin);
            printf("Button toggled!\n");

            if (xSemaphoreTake(conexaoMQTTSemaphore, portMAX_DELAY) &&
                    xSemaphoreTake(transmitDataSemaphore, portMAX_DELAY)) {
                char topic[50];
                char data[20];
                sprintf(topic, "fse2020/150009313/%s/estado", room);
                sprintf(data, "{\"s\": \"%d\"}", state);
                mqtt_send_message(topic, data);
                xSemaphoreGive(conexaoMQTTSemaphore);
                xSemaphoreGive(transmitDataSemaphore);
            }

            vTaskDelay(50 / portTICK_PERIOD_MS);
            gpio_isr_handler_add(pin, &gpio_isr_handler, (void *)pin);
        }
    }
}

void handleMessage(char *message) {
    cJSON *data = cJSON_Parse(message);
    cJSON *toggle = cJSON_GetObjectItem(data, "toggle");
    if (toggle) {
        toggle_led();
        char topic[50];
        char data[20];
        sprintf(topic, "fse2020/150009313/%s/estado", room);
        sprintf(data, "{\"o\": \"%d\"}", get_led_status());
        mqtt_send_message(topic, data);
    } else {
        char *room_value;
        cJSON *room_item = cJSON_GetObjectItem(data, "room");
        if (room_item) {
            room_value = room_item->valuestring;
            xSemaphoreGive(transmitDataSemaphore);
            sprintf(room, "%s", room_value);
            char sub[50];
            sprintf(sub, "fse2020/150009313/%s/output", room_value);
            mqtt_subscribe(sub);
            write_room_data(room_value);
            printf("%s\n", room_value);
        }
    }
    cJSON_Delete(data);
}

void readSensors(void *params) {
    char tdata[20];
    char hdata[20];
    char ttopic[50];
    char htopic[50];

    while (1) {
        read_sensor(&weather_data);
        printf("t: %d\n", weather_data.temperature);
        printf("h: %d\n", weather_data.humidity);

        if  (xSemaphoreTake(conexaoMQTTSemaphore, portMAX_DELAY) && xSemaphoreTake(transmitDataSemaphore, portMAX_DELAY)) {
            sprintf(tdata, "{\"t\": \"%d\"}", weather_data.temperature);
            sprintf(ttopic, "fse2020/150009313/%s/temperatura", room);
            mqtt_send_message(ttopic, tdata);
            sprintf(hdata, "{\"h\": \"%d\"}", weather_data.humidity);
            sprintf(htopic, "fse2020/150009313/%s/umidade", room);
            mqtt_send_message(htopic, hdata);

            xSemaphoreGive(conexaoMQTTSemaphore);
            xSemaphoreGive(transmitDataSemaphore);
        }

        vTaskDelay(30000 / portTICK_PERIOD_MS);
    }
}

void setupMqtt(void * params) {
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
            } else {
                char sub[50];
                sprintf(sub, "fse2020/150009313/%s/output", room);
                mqtt_subscribe(sub);
                xSemaphoreGive(transmitDataSemaphore);
            }
        }
    }
}