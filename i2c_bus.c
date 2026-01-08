#include "i2c_bus.h"
#include "hardware/gpio.h"
#include <stdio.h>

void i2c_bus_init(const i2c_bus_t *bus){
    i2c_init(bus->port, bus->freq_hz);

    gpio_set_function(bus->sda_pin, GPIO_FUNC_I2C);
    gpio_set_function(bus->scl_pin, GPIO_FUNC_I2C);

    gpio_pull_up(bus->sda_pin);
    gpio_pull_up(bus->scl_pin);
}

int i2c_bus_scan(const i2c_bus_t *bus){
    int count = 0;
    printf("\nStarting i2c scan.");
    
    //ret = common practice shorthand in c for "returned value"
    for (uint8_t addr = 0x08; addr <= 0x77; addr++) { 
        uint8_t dummy; //empty integer that takes up 8 bits of memory
        int ret = i2c_read_blocking(bus->port, addr, &dummy, 1, false); //read returned value into the dummy's memory location
        if (ret >= 0) {
            printf("\nFound device at 0x%02X\n", addr);
            count++;
        }
    }

    printf("\nFinished i2c scan.\n");   
    return count;
}