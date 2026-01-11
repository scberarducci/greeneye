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
#include "veml7700.h"
#include "ssd1306.h"

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
veml7700_t veml;
ssd1306_t oled;

int main() {
    stdio_init_all();
    sleep_ms(2000);

    //init i2c
    i2c_bus_init(&BUS0);
    int num_devices = i2c_bus_scan(&BUS0); //make sure there's devices on the bus
    if(num_devices == 0){printf("No devices found"); exit(1);}

    //init and config sensors, wifi, screen
    aht20_init(&aht, BUS0.port);

    veml7700_init(&veml, BUS0.port);
    if(!veml7700_config(&veml, GAIN_1x, ITIME_100MS)){
        printf("VEML7700 config failed");
    }

    printf("%d", ssd1306_init(&oled, BUS0.port, SSD1306_ADDR_0x3C));

    if (cyw43_arch_init() != 0) {
        // WiFi chip init failed -> LED control won't work
        printf("Wifi chip init failed");
        exit(1);
    }

    ssd1306_draw_string(&oled, 8, 0, "SCORE:4/8", 2);
    ssd1306_draw_string(&oled, 24,24, "Too humid", 1);
    ssd1306_draw_string(&oled, 24,34, "Too hot", 1);
    ssd1306_draw_string(&oled, 24,44, "Too dark", 1);
    ssd1306_show(&oled);

    while (true) {

        //-------- TEXT TEST CODE ---------

        

        //---------------------------------
        
        /*
        //--------- OLED TEST CODE ---------

        
        ssd1306_clear_buffer(&oled);
        ssd1306_show(&oled);
        for (int i = 0; i < 64; i++) {
            ssd1306_draw_pixel(&oled, i, i, true);
            ssd1306_show(&oled);
        }
        ssd1306_clear_buffer(&oled);
        ssd1306_show(&oled);
        for (int i = 0; i < 64; i++) {
            ssd1306_draw_pixel(&oled, 64-i, 64-i, true);
            ssd1306_show(&oled);
        }
        
        //------------------------------------
        */


        /*
        //------- VEML7700 TEST CODE --------

        uint16_t raw_counts;
        if(veml7700_read_counts(&veml, &raw_counts)){
            printf("\nRaw: %u", raw_counts);
        }
        else{
            printf("\nVEML7700 count read failed");
        }

        float lux;
        if(veml7700_read_lux_autorange(&veml, &lux)){
            printf("\nLux: %f", lux);
        }
        else{
            printf("\nVEML7700 lux read failed");
        }

        sleep_ms(1000);

        //-----------------------------------
        */


        /*
        //--------- AHT20 TEST CODE ---------

        float temp, humidity;

        if (aht20_read(&aht, &temp, &humidity)) {
            printf("Temp: %.1f C   RH: %.1f %%\n", temp, humidity);
        } else {
            printf("AHT20 read failed\n");
        }

        sleep_ms(1000);

        //-----------------------------------
        */
    }
}
