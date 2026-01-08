#pragma once
#include <stdint.h>
#include "hardware/i2c.h"

typedef struct {
    i2c_inst_t *port; //bus line
    uint sda_pin; //data gpio
    uint scl_pin; //clock gpio
    uint32_t freq_hz; //frequency in hz
} i2c_bus_t;

void i2c_bus_init(const i2c_bus_t *bus);

int i2c_bus_scan();