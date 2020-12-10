#include <stdio.h>

#include "dht11.h"
#include "climate.h"
#include "driver/gpio.h"

void init_sensor() {
    DHT11_init(GPIO_NUM_4);
}

void read_sensor(Weather_t *weather_data) {
    weather_data->temperature =  DHT11_read().temperature;
    weather_data->humidity =  DHT11_read().humidity;
}