
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "cJSON.h"


#include "control.h"
#include "button.h"
#include "led.h"

xQueueHandle interruptQueue;

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
            if (state == 1) {
                gpio_isr_handler_remove(pin);
                debounce();

                printf("Button toggled!\n");

                vTaskDelay(50 / portTICK_PERIOD_MS);
                gpio_isr_handler_add(pin, &gpio_isr_handler, (void *)pin);
            }
        }
    }
}

void handleMessage(char *message) {
    cJSON *data = cJSON_Parse(message);
    cJSON *toggle = cJSON_GetObjectItem(data, "toggle");
    if (toggle) {
        set_led_state(toggle->valueint);
    } else {
        char *room_value;
        room_value = cJSON_GetObjectItem(data, "room")->valuestring;
        printf("%s\n", room_value);
    }
}