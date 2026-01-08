#include "i2c_bus.h"
#include "hardware/gpio.h"

void i2c_bus_init(const i2c_bus_t *bus){
    i2c_init(bus->port, bus->freq_hz);

    gpio_set_function(bus->sda_pin, GPIO_FUNC_I2C);
    gpio_set_function(bus->scl_pin, GPIO_FUNC_I2C);

    gpio_pull_up(bus->sda_pin);
    gpio_pull_up(bus->scl_pin);
}