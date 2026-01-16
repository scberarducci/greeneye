/*
----------------------------------------------------
File: GREENEYE MAIN
Author: Sara Berarducci
Start Date: 01/07/2026
Description: Plant-monitoring embedded systems project

Last updated: 1/15/2026

----------------------------------------------------
REMINDERS:
-CTRL+A -> K -> Y to close serial output
-nano ~/.zshrc to see/add command shortcuts
-save main before flashing
----------------------------------------------------
*/

//standard headers to program pico
#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "cyw43.h"

//hardware headers (i2c, gpios)
#include "hardware/i2c.h"
#include "hardware/gpio.h"
#include "hardware/pio.h"
#include "ws2812.h"

//drivers and custom modules
#include "i2c_bus.h"
#include "aht20.h"
#include "veml7700.h"
#include "ssd1306.h"
#include "pec11r.h"

//default i2c settings
#define I2C_PORT i2c0
#define SDA_PIN  4
#define SCL_PIN  5

//default pio settings
#define WS2812_PIN 9
#define WS2812_NUM_PIXELS 8
#define WS2812_SM 0

//define LED strip used colors
#define RED {255,0,0}
#define YELLOW {255,255,0}
#define GREEN {0,255,0}
#define OFF {0,0,0}

//declare i2c bus
static const i2c_bus_t BUS0 = {
    .port = I2C_PORT,
    .sda_pin = SDA_PIN,
    .scl_pin = SCL_PIN,
    .freq_hz = 100 * 1000
};

//declare peripherals
aht20_t aht;
veml7700_t veml;
ssd1306_t oled;
pec11r_t enc;
led_strip_t strip;

int main() {

    stdio_init_all();
    sleep_ms(2000);

    //init i2c
    i2c_bus_init(&BUS0);
    int num_devices = i2c_bus_scan(&BUS0); //make sure there's devices on the bus
    if(num_devices == 0){printf("No devices found"); exit(1);}

    //init and config sensors, wifi, screen, encoder
    aht20_init(&aht, BUS0.port);

    veml7700_init(&veml, BUS0.port);
    if(!veml7700_config(&veml, GAIN_1x, ITIME_100MS)){
        printf("VEML7700 config failed");
    }

    printf("%d", ssd1306_init(&oled, BUS0.port, SSD1306_ADDR_0x3C));

    if (cyw43_arch_init() != 0) {
        // WiFi chip init failed -> LED control, bluetooth won't work
        printf("Wifi chip init failed");
    }

    pec11r_init(&enc, 6, 7, 8); //gpios 6,7 for rotary, gpio 8 for switch
    int enc_pos = 0;

    if(!led_strip_init(&strip, pio0, WS2812_SM, WS2812_PIN, WS2812_NUM_PIXELS)){
        printf("LED strip (PIO) init failed");
    }


    while (true) {
        
        //--------- MAIN CODE ----------

        char *plantname = "PLANTNAME";

        //------- VEML7700 CODE --------

        uint16_t raw_counts;
        char luxstr[32];
        if(veml7700_read_counts(&veml, &raw_counts)){
            printf("\nRaw: %u", raw_counts);
        }
        else{
            printf("\nVEML7700 count read failed");
        }

        float lux;
        if(veml7700_read_lux_autorange(&veml, &lux)){
            printf("\nLux: %f", lux);

            
            //do some calculations based on plant preset here
            //determine qualitative test for string
            

            snprintf(luxstr, sizeof(luxstr), "Too dark: %.0f lx", lux);
        }
        else{
            printf("\nVEML7700 lux read failed");
            snprintf(luxstr, sizeof(luxstr), "LUX READ ERR", lux);
        }

        //-----------------------------------
        
        
        //--------- AHT20 CODE ---------

        float temp, humidity;
        char tempstr[32], humstr[32];

        if (aht20_read(&aht, &temp, &humidity)) {
            printf("\nTemp: %.1f C   RH: %.1f %%\n", temp, humidity);

            
            //do some calculations based on plant preset here
            //determine qualitative text for strings
            
           
            snprintf(tempstr, sizeof(tempstr), "Too cold: %.1f C", temp);
            snprintf(humstr, sizeof(humstr), "Too humid: %.1f%%", humidity);

        } else {
            printf("AHT20 read failed\n");
            snprintf(tempstr, sizeof(tempstr), "HUM/TEMP ERR");
            snprintf(humstr, sizeof(humstr), "HUM/TEMP ERR");
        }

        //-----------------------------------
        

        //-------- CALCULATE SCORE ----------

        char scorestr[32];
        int score;

        
        //do some math here with temp, humidity, lux, and plant presets to determine an /10 score
        

       score = 4; //placeholder value
       snprintf(scorestr, sizeof(scorestr), "Score: %d/10", score);

       //----------------------------------


        //-------- PRINT TO OLED ---------

        sleep_ms(1000);
        ssd1306_clear_buffer(&oled);
        ssd1306_draw_string(&oled, 0, 0, plantname, 2, TRUNCATE);
        ssd1306_draw_string(&oled, 0,20, humstr, 1, TRUNCATE);
        ssd1306_draw_string(&oled, 0,30, tempstr, 1, TRUNCATE);
        ssd1306_draw_string(&oled, 0,40, luxstr, 1, TRUNCATE);
        ssd1306_draw_string(&oled, 0, 50, scorestr, 1, TRUNCATE);
        ssd1306_show(&oled);

        //---------------------------------
        

        //---------------------------------
        

    }
}


/*
ARCHIVED TEST CODE:

        //--------- OLED TEST CODE ---------

        
        ssd1306_clear_buffer(&oled);
        ssd1306_show(&oled);
        for (int i = 0; i < 64; i++) {
            ssd1306_draw_pixel(&oled, i, i, true);
            ssd1306_show(&oled);
        }
        ssd1306_clear_buffer(&oled);
        ssd1306_show(&oled);
        for (int i = 0; i < 64; i++) {
            ssd1306_draw_pixel(&oled, 64-i, 64-i, true);
            ssd1306_show(&oled);
        }
        
        //------------------------------------

        //------ ENCODER TEST CODE -------

        int click = pec11r_detent_poll(&enc);
        enc_pos += click;
        if(click!=0) printf("pos=%d\n", enc_pos);

        bool pressed = pec11r_sw_pressed(&enc);
        if(pressed) printf("button pressed");
        sleep_ms(1);

        //--------------------------------

        //-------- LED STRIP TEST CODE ----------

        led_strip_clear(&strip);
        uint32_t buf[][3] = {OFF, GREEN, GREEN, YELLOW, YELLOW, YELLOW, RED, RED};
        led_strip_set_buffer_rgb(&strip, buf,8);
        led_strip_show(&strip);

        //---------------------------------------
        
*/