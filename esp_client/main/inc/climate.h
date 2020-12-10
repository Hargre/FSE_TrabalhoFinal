#ifndef WEATHER_H
#define WEATHER_H

typedef struct Weather_t {
    int temperature;
    int humidity;
} Weather_t;

#define DHT_GPIO 4

void init_sensor();
void read_sensor(Weather_t *data);

#endif