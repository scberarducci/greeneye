#include "pec11r.h"
#include "hardware/gpio.h"
#include "pico/stdlib.h"

void pec11r_init(pec11r_t *dev, uint32_t gpio_a, uint32_t gpio_b, uint32_t gpio_sw){
    dev->gpio_a = gpio_a;
    dev->gpio_b = gpio_b;
    dev->gpio_sw = gpio_sw;

    gpio_init(gpio_a);
    gpio_set_dir(gpio_a, GPIO_IN);
    gpio_pull_up(gpio_a);

    gpio_init(gpio_b);
    gpio_set_dir(gpio_b, GPIO_IN);
    gpio_pull_up(gpio_b);

    if (gpio_sw != UINT32_MAX) { //button might not be used
        gpio_init(gpio_sw);
        gpio_set_dir(gpio_sw, GPIO_IN);
        gpio_pull_up(gpio_sw);

        dev->sw_prev = gpio_get(gpio_sw); // raw switch value
        dev->last_sw_change_us = time_us_32();
        dev->sw_held = !dev->sw_prev; //if switch is low, then is currently held
    }

    uint8_t a = gpio_get(gpio_a) ? 1 : 0;
    uint8_t b = gpio_get(gpio_b) ? 1 : 0;
    dev->ab_prev = (uint8_t)((a << 1) | b); //lowest two bits represent direction of rotation

    

    dev->edge_accum = 0;
    dev->edge_per_detent = 4;
}

//read and return raw ab value
uint8_t pec11r_read_ab(const pec11r_t *dev){
    uint8_t a = gpio_get(dev->gpio_a) ? 1 : 0;
    uint8_t b = gpio_get(dev->gpio_b) ? 1 : 0;
    return (uint8_t)((a << 1) | b);
}

//read switch value
bool pec11r_sw_pressed(pec11r_t *dev) {
    if (dev->gpio_sw == UINT32_MAX) { return false; }

    uint32_t now = time_us_32();
    bool sw_curr = gpio_get(dev->gpio_sw);

    if (dev->sw_prev != sw_curr){ //if state changed
        if(now - dev->last_sw_change_us >= 20000){ //20 ms debounce
            dev->sw_prev = sw_curr;
            dev->last_sw_change_us = now;
            if (sw_curr == 0){ //if falling edge from not-presssed -> pressed
                if(!dev->sw_held){
                    dev->sw_held = true;
                    return true;
                } 
            } else {
                dev->sw_held = false;
            }
        }
    } 

    return (false);
}

//from previous and current ab values, what is the encoder direction? See below for values
static const int8_t enc_dir_lut[16] ={
    //prev = 00
    0, +1, -1, 0,
    //prev = 01
    -1, 0, 0, +1,
    //prev = 10
    +1, 0, 0, -1,
    //prev = 11
    0, -1, +1, 0
};
/*
------------ LUT KEY -------------
    prev\curr   00	01	10	11
        00	    0	+1	−1	0
        01	    −1	0	0	+1
        10	    +1	0	0	−1
        11	    0	−1	+1	0
----------------------------------
*/

//returns 1, 0, -1 for direction (CW, no change, CCW)
int pec11r_update(pec11r_t *dev){
    uint8_t curr = pec11r_read_ab(dev);
    uint8_t index = (uint8_t)((dev->ab_prev<<2) | curr);

    int8_t step = enc_dir_lut[index];

    dev->ab_prev = curr;
    return (int)step;
}

int pec11r_detent_poll(pec11r_t *dev){ //return change only after full detent
    int step = pec11r_update(dev);
    if(step == 0){ return 0; }

    dev->edge_accum += step;

    if (dev->edge_accum >= dev->edge_per_detent){
        dev->edge_accum = 0;
        return +1;
    }
    if (dev->edge_accum <= -dev->edge_per_detent){
        dev->edge_accum = 0;
        return -1;
    }
    return 0;
}