#include "veml7700.h"
#include "hardware/i2c.h" //for i2c implementation
#include "pico/stdlib.h" //for timing
#include <stdio.h> //DEBUG ONLY

#define VEML7700_ADDR 0x10

//to later clear gain and itime config bits before writing new ones
#define GAIN_MASK (3<<11)
#define ITIME_MASK (15<<11)

//define autorange values
#define SATURATION 60000
#define NOISE_FLOOR 2000
veml7700_mode_t AUTORANGE_SETTINGS[] = {
    {GAIN_2x, ITIME_800MS}, //most sensitive
    {GAIN_2x, ITIME_400MS},
    {GAIN_2x, ITIME_200MS},
    {GAIN_1x, ITIME_200MS},
    {GAIN_1x, ITIME_100MS}, //default
    {GAIN_1_04x, ITIME_100MS},
    {GAIN_1_08x, ITIME_100MS},
    {GAIN_1_08x, ITIME_50MS},
    {GAIN_1_08x, ITIME_25MS} //least sensitive
};

//define registers that need to be accessible
#define VEML7700_CONFIG_REG 0x00
#define VEML7700_OUTPUT_REG 0x04

//helpers to map gain settings to bits foro configuration
static uint16_t gain_to_bits(veml7700_gain_t gain){
    switch(gain){
        case GAIN_1x: return (0<<11);
        case GAIN_2x: return (1<<11);
        case GAIN_1_04x: return (2<<11);
        case GAIN_1_08x: return (3<<11);
        default: return (0<<11);
    }
}
static uint16_t itime_to_bits(veml7700_itime_t itime_ms){
    switch(itime_ms){
        case ITIME_25MS: return (12<<6);
        case ITIME_50MS: return (8<<6);
        case ITIME_100MS: return (0<<6);
        case ITIME_200MS: return (1<<6);
        case ITIME_400MS: return (2<<6);
        case ITIME_800MS: return (3<<6);
        default: return (0<<6);
    }
}


//initialize veml port and address
void veml7700_init(veml7700_t *dev, i2c_inst_t *port){
    dev->port = port;
    dev->addr = VEML7700_ADDR;
}

//configure initial gain and integration time settings
bool veml7700_config(veml7700_t *dev, veml7700_gain_t gain, veml7700_itime_t itime_ms){
    dev->gain = gain;
    dev->itime_ms = itime_ms;

    //create bitwise configuration
    uint16_t config = gain_to_bits(gain) | itime_to_bits(itime_ms);
    //create message to send over I2C: write to config register 0, low byte, high byte
    uint8_t buffer[3] = {VEML7700_CONFIG_REG, (uint8_t)(config & 0xFF), (uint8_t)((config>>8) & 0xFF)};
    //send the message; save number of bits successfully written
    int write = i2c_write_blocking(dev->port,dev->addr,buffer,3,false);
    //return success
    return (write == 3);
}

//reads raw data from sensor
bool veml7700_read_counts(const veml7700_t *dev, uint16_t *counts){
    //request register
    uint8_t reg_buffer = VEML7700_OUTPUT_REG;
    int write = i2c_write_blocking(dev->port, dev->addr, &reg_buffer, 1, true);
    if(write != 1){return false;}

    //read register's data
    uint8_t buffer[2];
    int data = i2c_read_blocking(dev->port,dev->addr, buffer,2,false);
    if(data != 2){return false;}

    //change counts to reflect read data
    *counts = ((uint16_t)(buffer[1]) << 8)|(uint16_t)(buffer[0]);
    return true; //return success
}

//helper function; calculates lux per count based on gain
static float veml7700_lux_per_count(const veml7700_t *dev){
    float gain_val, itime_val = (dev->itime_ms);
    if (itime_val == 0) {return 0.0f;}
    switch(dev->gain){
        case GAIN_1x: gain_val = 1.0f;
        case GAIN_2x: gain_val = 2.0f;
        case GAIN_1_04x: gain_val = 0.25f;
        case GAIN_1_08x: gain_val = 0.125f;
        default: gain_val = 1.0f;
    }
    //reference from datasheet: 0.0042 lux/count at gain = 2x, itime = 800ms
    return 0.0042f * (800.0f / itime_val) * (2.0f / gain_val);
}

//reads and returns lux using internal helper functions
bool veml7700_read_lux(const veml7700_t *dev, float *lux){
    uint16_t counts;
    if(!veml7700_read_counts(dev, &counts)){
        return false;
    }

    *lux = (float)counts * veml7700_lux_per_count(dev);
    return true;
}

//helper function; updates gain and integration time settings for maximum accuracy
static bool veml7700_autorange_update(veml7700_t *dev, uint16_t counts){
    veml7700_mode_t curr_mode = {dev->gain, dev->itime_ms};
    int len = sizeof(AUTORANGE_SETTINGS)/sizeof(AUTORANGE_SETTINGS[0]);
    for(int i = 0; i < len; i++){
        if(curr_mode.gain == AUTORANGE_SETTINGS[i].gain && curr_mode.itime_ms == AUTORANGE_SETTINGS[i].itime_ms){
            if(counts > SATURATION && i+1 < len){
                return veml7700_config(dev, AUTORANGE_SETTINGS[i+1].gain, AUTORANGE_SETTINGS[i+1].itime_ms);
            }
            else if (counts < NOISE_FLOOR && i > 0){
                return veml7700_config(dev, AUTORANGE_SETTINGS[i-1].gain, AUTORANGE_SETTINGS[i-1].itime_ms);
            }
            return false;
        }
    }
    return false;
}

//wrapper for reading lux and adjusting gain/IT accordingly
bool veml7700_read_lux_autorange(veml7700_t *dev, float *lux){
    uint16_t counts;
    if(!veml7700_read_counts(dev, &counts)){
        return false;
    }

    if(veml7700_autorange_update(dev, counts)){
        uint16_t throwaway;
        sleep_ms(dev->itime_ms);
        printf("\nUPDATED");
        veml7700_read_counts(dev, &throwaway);
        sleep_ms(1000);
        if(!veml7700_read_counts(dev, &counts)){
            return false;
        }
    }

    *lux = (float)counts * veml7700_lux_per_count(dev);;

    return true;
}
