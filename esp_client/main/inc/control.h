#ifndef CONTROL_H
#define CONTROL_H

#include "freertos/FreeRTOS.h"

void IRAM_ATTR gpio_isr_handler(void *args);
void setupButtonHandler();
void buttonHandler(void *params);
void handleMessage(char *message);

#endif