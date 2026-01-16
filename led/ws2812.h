#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "hardware/pio.h"

typedef struct {
    PIO pio;          // pio0 or pio1
    uint32_t sm;          // state machine index 0..3
    uint32_t pin;         // data GPIO
    uint32_t num_px;       // number of LEDs

    uint32_t *pixels; //buffer
} led_strip_t;

//init using pio program and state machine with given params
bool led_strip_init(led_strip_t *dev, PIO pio, uint32_t sm, uint32_t pin, uint32_t count); 

//fill strip with one color
void led_strip_fill_rgb(const led_strip_t *dev, uint8_t r, uint8_t g, uint8_t b);

//set one pixel's color in buffer
void led_strip_set_rgb(led_strip_t *dev, uint index, uint8_t r, uint8_t g, uint8_t b);

void led_strip_set_buffer_rgb(const led_strip_t *dev, uint32_t (*buffer)[3], uint32_t len);

//show current buffer
void led_strip_show(const led_strip_t *dev);

//clear buffer
void led_strip_clear(const led_strip_t *dev);