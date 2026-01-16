#include "aht20.h" //actual driver
#include "hardware/i2c.h" //for i2c implementation
#include "pico/stdlib.h" //for timing

#define AHT20_ADDR 0x38

static const uint8_t AHT20_TRIGGER[3] = {0xAC, 0x33, 0x00};

void aht20_init(aht20_t *dev, i2c_inst_t *port){
    dev->port = port; //sets i2c bus
    dev->addr = AHT20_ADDR; //sets address
}

bool aht20_read(const aht20_t *dev, float *temp_c, float *humidity_perc){
    int write = i2c_write_blocking(dev->port, dev->addr,AHT20_TRIGGER,3,false); //write command
    if(write != 3){ return false; } //error case (write)

    sleep_ms(80);

    uint8_t data[6] = {0}; //buffer
    int read = i2c_read_blocking(dev->port, dev->addr, data, 6, false); //read sensor data
    if(read!=6){ return false; } //error case (read)

    uint32_t raw_h = //create raw data humidity reading from hex array
        ((uint32_t)data[1] << 12) |
        ((uint32_t)data[2] << 4) |
        ((data[3] >> 4) & 0x0F);
    *humidity_perc = (raw_h * 100.0f) / 1048576.0f; //convert to human-readable humidity

    uint32_t raw_t =  //create raw data temp reading from hex array
        ((uint32_t)(data[3] & 0x0F) << 16) |
        ((uint32_t)data[4] << 8) |
        (uint32_t)data[5];
    *temp_c = (raw_t * 200.0f) / 1048576.0f - 50.0f; //convert to human-readable temp (degree C)

}