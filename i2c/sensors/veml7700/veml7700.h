#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "hardware/i2c.h"

typedef enum { //gain
    GAIN_1x = 1, //1x gain
    GAIN_2x = 2, //2x gain
    GAIN_1_04x = 4, //1/4 (0.25)x gain
    GAIN_1_08x = 8 //1/8 (0.125)x gain
} veml7700_gain_t;

typedef enum{ //integration time
    ITIME_25MS = 25, //25 milliseconds
    ITIME_50MS = 50, //50 ms
    ITIME_100MS = 100, //etc.
    ITIME_200MS = 200,
    ITIME_400MS = 400,
    ITIME_800MS = 800
} veml7700_itime_t;

typedef struct {
    i2c_inst_t *port;
    uint8_t addr;
    veml7700_gain_t gain;
    veml7700_itime_t itime_ms;
} veml7700_t;

//initialize veml port and address
void veml7700_init(veml7700_t *dev, i2c_inst_t *port);

//configure initial gain and integration time settings
bool veml7700_config(veml7700_t *dev, veml7700_gain_t gain, veml7700_itime_t itime_ms);

//helper function; calculates lux per count based on gain
static float veml7700_lux_per_count(const veml7700_t *dev);

//reads raw data from sensor
bool veml7700_read_counts(const veml7700_t *dev, uint16_t *counts);

//reads and returns lux using internal helper functions; update gain and integration time settings based on reading
bool veml7700_read_lux_autorange(const veml7700_t *dev, float *lux);

