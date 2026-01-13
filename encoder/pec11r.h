#pragma once
#include <stdint.h>
#include <stdbool.h> 

typedef struct{
    uint32_t gpio_a;
    uint32_t gpio_b;
    uint32_t gpio_sw;

    uint8_t ab_prev;
    
    bool sw_prev;
    uint32_t last_sw_change_us;
    bool sw_held;

    int8_t edge_accum;
    uint8_t edge_per_detent;
} pec11r_t;

void pec11r_init(pec11r_t *dev, uint32_t gpio_a, uint32_t gpio_b, uint32_t gpio_sw);

uint8_t pec11r_read_ab(const pec11r_t *dev); //0-3, only 2 bits used

bool pec11r_sw_pressed(pec11r_t *dev);

int pec11r_update(pec11r_t *dev);

int pec11r_detent_poll(pec11r_t *dev);
