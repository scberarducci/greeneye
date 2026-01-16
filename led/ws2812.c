#include "ws2812.h"
#include "ws2812.pio.h"
#include <stdlib.h>
#include "pico/stdlib.h"

#define MAX_BRIGHTNESS 0.05 //5% brightness max
#define BRIGHTNESS_SCALE MAX_BRIGHTNESS*255

static PIO ws2812_pio = pio0;

//humans think in rgb, ws2812 wants grb
static inline uint32_t rgb_to_grb(uint8_t r, uint8_t g, uint8_t b) {
    r = (uint8_t)((r*BRIGHTNESS_SCALE)/255);
    g = (uint8_t)((g*BRIGHTNESS_SCALE)/255);
    b = (uint8_t)((b*BRIGHTNESS_SCALE)/255);

    return (((uint32_t)g << 16) | ((uint32_t)r << 8) | (uint32_t)b);
}

//directly writes to LED strip; no buffer
static inline void put_pixel(const led_strip_t *dev, uint32_t grb) {
    pio_sm_put_blocking(dev->pio, dev->sm, grb << 8u); //leftshift is because pio expects 32 bit words, but each pixel consumes just top 24 bits
}

bool led_strip_init(led_strip_t *dev, PIO pio, uint32_t sm, uint32_t pin, uint32_t num_px) {
    if (!dev) return false;

    dev->pio = pio;
    dev->sm = sm;
    dev->pin = pin;
    dev->num_px = num_px;

    dev->pixels = calloc(num_px, sizeof(uint32_t));
    if(!dev->pixels){ return false; } //in case buffer not allocated

    uint32_t offset = pio_add_program(pio, &ws2812_program);

    ws2812_program_init(
        pio,
        sm,
        offset,
        pin,
        800000,  // 800kHz; PIO decides timing efficiently
        false    // RGB (not RGBW)
    );

    // Start with LEDs off
    led_strip_fill_rgb(dev, 0, 0, 0);
    return true;
}

void led_strip_fill_rgb(const led_strip_t *dev, uint8_t r, uint8_t g, uint8_t b) {
    if (!dev) return;
    uint32_t grb = rgb_to_grb(r, g, b);
    for (uint32_t i = 0; i < dev->num_px; i++) {
        dev->pixels[i] = grb;
    }
}

void led_strip_set_rgb(led_strip_t *dev, uint index, uint8_t r, uint8_t g, uint8_t b){
    if (!dev || index >= dev->num_px) return;
    dev->pixels[index] = rgb_to_grb(r, g, b);
}

void led_strip_set_buffer_rgb(const led_strip_t *dev, uint32_t (*buffer)[3], uint32_t len){
    if (!dev || sizeof(buffer)/sizeof(buffer[0]) >= dev->num_px) return;

    for(int i = 0; i < len; i++){
        dev->pixels[i] = rgb_to_grb(buffer[i][0],buffer[i][1],buffer[i][2]);
    }
}

void led_strip_show(const led_strip_t *dev) {
    if (!dev) return;

    for (uint i = 0; i < dev->num_px; i++) {
        put_pixel(dev, dev->pixels[i]); // pushes buffer data to hardware
    }
    sleep_us(80);
}

void led_strip_clear(const led_strip_t *dev) {
    if (!dev) return;
    for (uint i = 0; i < dev->num_px; i++) {
        dev->pixels[i] = 0;
    }
}