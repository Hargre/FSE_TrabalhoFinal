#include "driver/gpio.h"

#include "button.h"
#include "control.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void init_button() {
    gpio_pad_select_gpio(BUTTON);
    gpio_set_direction(BUTTON, GPIO_MODE_INPUT);
    gpio_pulldown_en(BUTTON);
    gpio_pullup_dis(BUTTON);

    gpio_set_intr_type(BUTTON, GPIO_INTR_ANYEDGE);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(BUTTON, &gpio_isr_handler, (void *)BUTTON);
}

void debounce(int state) {
    while(gpio_get_level(BUTTON) == state) {
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}