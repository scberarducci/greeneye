#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "hardware/i2c.h"

typedef struct {
    i2c_inst_t *port;
    uint8_t addr;
} aht20_t;

void aht20_init(aht20_t *dev, i2c_inst_t *port);

bool aht20_read(const aht20_t *dev, float *temp_c, float *humidity_perc);

