#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "led.h"

void init_led() {
    gpio_pad_select_gpio(LED);
    gpio_set_direction(LED, GPIO_MODE_INPUT_OUTPUT);
}

void blink_led() {
    gpio_set_level(LED, 0);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    gpio_set_level(LED, 1);
}

void toggle_led() {
    int state = gpio_get_level(LED);
    gpio_set_level(LED, !state);
}

int get_led_status() {
    return gpio_get_level(LED);
}