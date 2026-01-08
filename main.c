/*
----------------------------------------------------
File: GREENEYE MAIN
Author: Sara Berarducci
Date: 01/07/2026
Description: Plant-monitoring embedded systems project

Last updated: 1/07/2026

----------------------------------------------------
REMINDERS:
-CTRL+A -> K -> Y to close serial output
-nano ~/.zshrc to see/add command shortcuts
-save main before flashing
----------------------------------------------------
*/

//standard headers to program pico
#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "cyw43.h"

//hardware headers (i2c, gpios)
#include "hardware/i2c.h"
#include "hardware/gpio.h"

//drivers and custom modules
#include "i2c_bus.h"
#include "aht20.h"

//default i2c settings
#define I2C_PORT i2c0
#define SDA_PIN  4
#define SCL_PIN  5

//i2c addresses
#define AHT20_ADDR 0x38

//declare i2c bus
static const i2c_bus_t BUS0 = {
    .port = I2C_PORT,
    .sda_pin = SDA_PIN,
    .scl_pin = SCL_PIN,
    .freq_hz = 100 * 1000
};

//declare sensors
aht20_t aht;

int main() {
    stdio_init_all();
    sleep_ms(2000);

    //init i2c
    i2c_bus_init(&BUS0);
    int num_devices = i2c_bus_scan(&BUS0); //make sure there's devices on the bus
    if(num_devices == 0){printf("No devices found"); exit(1);}

    //init sensors
    aht20_init(&aht, BUS0.port);

    if (cyw43_arch_init() != 0) {
        // WiFi chip init failed -> LED control won't work
        printf("Wifi chip init failed");
        exit(1);
    }

    while (true) {
        float temp, humidity;

        if (aht20_read(&aht, &temp, &humidity)) {
            printf("Temp: %.1f C   RH: %.1f %%\n", temp, humidity);
        } else {
            printf("AHT20 read failed\n");
        }

        sleep_ms(1000);
    }
}
