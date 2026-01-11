#include "ssd1306.h"
#include <string.h>

//screen control codes
#define COMMAND 0x00 //control byte; tells device next info is command info
#define DATA 0x40 //control byte; tells device next info is to be displayed

#define DISPLAY_OFF 0xAE
#define DISPLAY_ON 0xAF

#define SET_MUX_RATIO 0xA8
#define SET_OFFSET 0xD3
#define START_LINE_0 0x40
#define SET_CHARGE_PUMP 0x8D
#define SET_MEMORY_MODE 0x20
#define OUTPUT_FROM_RAM 0xA4
#define NORMAL_DISPLAY_MODE 0xA6

#define DEFAULT_SCAN_V_DIR 0xC0 //yellow section at bottom
#define INVERTED_SCAN_V_DIR 0xC8 //yellow section at top
#define DEFAULT_SCAN_H_DIR 0xA0 //right to left
#define INVERTED_SCAN_H_DIR 0xA1 //left to right


//helper function to write a list of commands to the screen
static bool ssd1306_write_commands(const ssd1306_t *dev, const uint8_t *commands, size_t n){

    uint8_t temp[32]; //temporary buffer to hold commands
    if(n+1 > sizeof(temp) || !dev || !commands){ return false; } //if there are too many (or invalid input), don't get tied up

    temp[0] = COMMAND; //control byte; tells device next info is command info
    for(size_t i = 0; i < n; i++){
        temp[i+1] = commands[i]; //shift commands over one to make room for control byte
    }

    int write = i2c_write_blocking(dev->port, dev->addr, temp, (int)(n+1), false);

    return (write == (int)(n+1));
}

bool ssd1306_init(ssd1306_t *dev, i2c_inst_t *port, uint8_t addr){
    dev->port = port;
    dev->addr = addr;

    const uint8_t init_commands[] = {
        DISPLAY_OFF,            

        SET_MUX_RATIO, 0x3F,       // multiplex ratio = 64 (128x64 panels)
        SET_OFFSET, 0x00,       // display offset = 0
        START_LINE_0,             // start line = 0

        SET_CHARGE_PUMP, 0x14,       // charge pump enable
        SET_MEMORY_MODE, PAGE,       // memory mode = page

        OUTPUT_FROM_RAM,             // output from RAM, not overridden
        NORMAL_DISPLAY_MODE,             // normal display (not inverted)
        INVERTED_SCAN_V_DIR,      //scan direction from top (yellow) to bottom (blue)
        INVERTED_SCAN_H_DIR,     //scan direction from left to right with yellow on top 

        DISPLAY_ON             
    };
    ssd1306_write_commands(dev, init_commands, sizeof(init_commands));

    ssd1306_fill_buffer(dev);
    ssd1306_show(dev);
    sleep_ms(500);
    ssd1306_clear_buffer(dev);
    ssd1306_show(dev);

    return true;

}

void ssd1306_clear_buffer(ssd1306_t *dev){
    if (!dev) return;
    memset(dev->buffer, 0x00, BUFFER_SIZE);
}

void ssd1306_fill_buffer(ssd1306_t *dev){
    if (!dev) return;
    memset(dev->buffer, 0xFF, BUFFER_SIZE);
}

bool ssd1306_show(ssd1306_t *dev){ //directly replaces screen's RAM with buffer
    for (uint8_t page = 0; page < (DISPLAY_HEIGHT/8); page++) { //a page is a rectangle that spans the display horizontally and is 8 pixels high
        uint8_t set_page[] = { (uint8_t)(0xB0 | page) }; //0xB_ chooses a page; 0xB0 | page sets it to current page
        if (!ssd1306_write_commands(dev, set_page, sizeof(set_page))){ return false; } //moves "cursor" to current page

        uint8_t set_col[] = { 0x00, 0x10 }; //0x0_ is lower column register (lowest 4 bits), 0x1_ is upper column register (highest 3 bits) for 7 bit address
        if (!ssd1306_write_commands(dev, set_col, sizeof(set_col))){ return false; } //moves "cursor" to first column (col 0) in the page

        const uint8_t *src = &dev->buffer[page * DISPLAY_WIDTH]; //pointer to buffer array

        uint8_t data[1 + DISPLAY_WIDTH]; //1 control byte, DISPLAY_WIDTH (128) bytes of data per page
        data[0] = DATA; //first byte is control byte that says display data is coming
        for (int i = 0; i < DISPLAY_WIDTH; i++) { data[i+1] = src[i]; } // fill the rest with buffer data

        int write = i2c_write_blocking(dev->port, dev->addr, data, (int)sizeof(data), false); //write all 0s to screen
        if(write != (int)(sizeof(data))){ return false; }
    }
    return true;
}

//draws pixel IN THE BUFFER; still needs to be shown to send to OLED's RAM
void ssd1306_draw_pixel(ssd1306_t *dev, int x, int y, bool on){ 
    if( x<0 || x>= DISPLAY_WIDTH || y<0 || y>=DISPLAY_HEIGHT ){ return; } //check valid bounds

    int page = y/8; //finds page/rectangle; 8 pixels per page, 8 pages per screen
    int bit = y % 8; //finds vertical position in the page

    int index = (page * DISPLAY_WIDTH) + x; //finds position of byte that pixel is located in

    uint8_t mask = (uint8_t) 1u << bit;

    if (on) { 
        dev->buffer[index] |=  mask; // set bit
    } 
    else {
        dev->buffer[index] &= (uint8_t)~mask; // clear bit
    }
}

//helper function to write a letter to the screen
void ssd1306_draw_glyph(ssd1306_t *dev, int x, int y, const uint8_t c[], int rows, int cols, int scale)
{
    for (int row = 0; row < rows; row++) {
        uint8_t bits = c[row];

        for (int col = 0; col < cols; col++) {
            bool pixel_on = (bits >> col) & 1u;
            if (pixel_on) {
                ssd1306_draw_pixel(dev, x + col * scale, y + row * scale, true);
            }
        }
    }
}

