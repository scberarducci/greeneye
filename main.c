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
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "cyw43.h"

//hardware headers (i2c, gpios)
#include "hardware/i2c.h"
#include "hardware/gpio.h"

//default i2c settings
#define I2C_PORT i2c0
#define SDA_PIN  4
#define SCL_PIN  5

#define AHT20_ADDR 0x38

void i2c_scan(){
    printf("Starting i2c scan.");

    //uint8_t = unsigned integer of 8 bits (1 byte), data type
    //ret = common practice shorthand in c for "returned value"
    for (uint8_t addr = 0x08; addr <= 0x77; addr++) { 
        uint8_t dummy; //empty integer that takes up 8 bits of memory
        int ret = i2c_read_blocking(I2C_PORT, addr, &dummy, 1, false); //read returned value into the dummy's memory location
        if (ret >= 0) {
            printf("\nFound device at 0x%02X\n", addr);
        }
    }

    printf("Finished i2c scan.");   
}


int main() {
    stdio_init_all();
    sleep_ms(2000);

    i2c_init(I2C_PORT, 100 * 1000); // 100kHz
    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(SDA_PIN);
    gpio_pull_up(SCL_PIN);

    if (cyw43_arch_init() != 0) {
        // WiFi chip init failed -> LED control won't work
        while (true) {
            sleep_ms(1000);
            printf("wifi chip not initialized");
        }
    }

    i2c_scan();

    while (true) {
        uint8_t aht20_trigger[3] = {0xAC, 0x33, 0x00};
        int write = i2c_write_blocking(I2C_PORT, AHT20_ADDR,aht20_trigger,3,false);
        if(write != 3){
            printf("command failed");
            sleep_ms(1000);
            continue;
        }

        sleep_ms(80); //gives sensor time to read environment

        uint8_t data[6] = {0};
        int read = i2c_read_blocking(I2C_PORT, AHT20_ADDR, data, 6, false);
        if (read != 6){
            printf("unable to read from device");
            sleep_ms(1000);
            continue;
        }

        printf("raw: %02X %02X %02X %02X %02X %02X\n",
           data[0], data[1], data[2], data[3], data[4], data[5]);

        if (data[0] & 0x80) {
            printf("busy\n");
        }

        sleep_ms(1000);
    }
}
