#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "hardware/i2c.h"

/*
  Module target: GME12864-13 family (0.96", 128x64, I2C, addr 0x3C or 0x3D).
  - Board-level strap/jumper sets the I2C address
  - Controller is SSD1306-based; command set + page addressing apply
  - No reset pin exposed in specific module used so rely on power-on reset + init commands
*/

//properties of GME
#define DISPLAY_WIDTH 128
#define DISPLAY_HEIGHT 64
#define BUFFER_SIZE ((DISPLAY_WIDTH * DISPLAY_HEIGHT) / 8) //1 pixel per bit, 8 pixels per byte

//typical addresses for ssd1306 modules
//put in header file for public access and readability when passing
#define SSD1306_ADDR_0x3C 0x3C
#define SSD1306_ADDR_0x3D 0x3D

//types of addressing modes
typedef enum {
  HORIZONTAL = 0,
  VERTICAL = 1,
  PAGE = 2
} ssd1306_addrmode_t;

typedef struct {
  i2c_inst_t *port;
  uint8_t addr;

  uint16_t width;
  uint16_t height;
  uint8_t offset; //optional column offset to shift image horizontally

  uint8_t buffer[BUFFER_SIZE];
  
  //for writing text to screen
  uint8_t x_pos;
  uint8_t y_pos;
  bool text_wrap;
  bool invert;

} ssd1306_t;

bool ssd1306_init(ssd1306_t *dev, i2c_inst_t *port, uint8_t addr);

void ssd1306_clear_buffer(ssd1306_t *dev);

void ssd1306_fill_buffer(ssd1306_t *dev);

bool ssd1306_show(ssd1306_t *dev);

void ssd1306_draw_pixel(ssd1306_t *dev, int x, int y, bool on);

void ssd1306_draw_glyph(ssd1306_t *dev, int x, int y, const uint8_t c[], int rows, int cols, int scale);