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

xQueueHandle interruptQueue;
extern char room[15];
extern xSemaphoreHandle transmitDataSemaphore;
extern xSemaphoreHandle conexaoMQTTSemaphore;

void IRAM_ATTR gpio_isr_handler(void *args) {
    int pin = (int)args;
    xQueueSendFromISR(interruptQueue, &pin, NULL);
}

void setupButtonHandler() {
    interruptQueue = xQueueCreate(10, sizeof(int));
    xTaskCreate(buttonHandler, "ButtonHandler", 2048, NULL, 1, NULL);
}

void sendState(void *params) {
    
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