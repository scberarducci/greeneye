#include "veml7700.h"
#include "hardware/i2c.h" //for i2c implementation
#include "pico/stdlib.h" //for timing

#define VEML7700_ADDR 0x10

//to later clear gain and itime config bits before writing new ones
#define GAIN_MASK (3<<11)
#define ITIME_MASK (15<<11)

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
;
}

//reads and returns lux using internal helper functions
bool veml7700_read_lux(const veml7700_t *dev, float *lux){
;
}