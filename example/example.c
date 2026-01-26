#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "pico/stdlib.h"

#include "ssd1306.h"
#include "image.h"
#include "acme_5_outlines_font.h"
#include "bubblesstandard_font.h"
#include "crackers_font.h"
#include "BMSPA_font.h"

const uint8_t num_chars_per_disp[]={7,7,7,5};
const uint8_t *fonts[4]= {acme_font, bubblesstandard_font, crackers_font, BMSPA_font};

#define SLEEPTIME 25

/*
 * Pin definitions - adjust these for your hardware setup
 */
#ifdef SSD1306_USE_SPI
/* SPI pin definitions */
#define SPI_PORT spi0
#define PIN_SCK  18   /* SPI clock */
#define PIN_MOSI 19   /* SPI MOSI (data) */
#define PIN_DC   20   /* Data/Command */
#define PIN_RST  21   /* Reset */
#define PIN_CS   17   /* Chip Select (directly connect to GND if only one device) */
#else
/* I2C pin definitions */
#define I2C_PORT i2c1
#define PIN_SDA  2
#define PIN_SCL  3
#define I2C_ADDR 0x3C
#endif

void setup_gpios(void);
void animation(void);

int main() {
    stdio_init_all();

    printf("configuring pins...\n");
    setup_gpios();

    printf("jumping to animation...\n");
    animation();

    return 0;
}


void setup_gpios(void) {
#ifdef SSD1306_USE_SPI
    /* Initialize SPI at 1MHz */
    spi_init(SPI_PORT, 1000000);
    gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);
    
    /* Chip select - optional, can be tied to GND */
    gpio_init(PIN_CS);
    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_put(PIN_CS, 0);  /* Select the device */
#else
    /* Initialize I2C at 400kHz */
    i2c_init(I2C_PORT, 400000);
    gpio_set_function(PIN_SDA, GPIO_FUNC_I2C);
    gpio_set_function(PIN_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(PIN_SDA);
    gpio_pull_up(PIN_SCL);
#endif
}


void animation(void) {
    const char *words[]= {"SSD1306", "DISPLAY", "DRIVER"};

    ssd1306_t disp;
    disp.external_vcc=false;
    
#ifdef SSD1306_USE_SPI
    ssd1306_init(&disp, 128, 64, PIN_DC, PIN_RST, SPI_PORT);
#else
    ssd1306_init(&disp, 128, 64, I2C_ADDR, I2C_PORT);
#endif
    ssd1306_clear(&disp);

    printf("ANIMATION!\n");

    char buf[8];

    for(;;) {
        for(int y=0; y<31; ++y) {
            ssd1306_draw_line(&disp, 0, y, 127, y);
            ssd1306_show(&disp);
            sleep_ms(SLEEPTIME);
            ssd1306_clear(&disp);
        }

        for(int y=0, i=1; y>=0; y+=i) {
            ssd1306_draw_line(&disp, 0, 31-y, 127, 31+y);
            ssd1306_draw_line(&disp, 0, 31+y, 127, 31-y);
            ssd1306_show(&disp);
            sleep_ms(SLEEPTIME);
            ssd1306_clear(&disp);
            if(y==32) i=-1;
        }

        for(int i=0; i<sizeof(words)/sizeof(char *); ++i) {
            ssd1306_draw_string(&disp, 8, 24, 2, words[i]);
            ssd1306_show(&disp);
            sleep_ms(800);
            ssd1306_clear(&disp);
        }

        for(int y=31; y<63; ++y) {
            ssd1306_draw_line(&disp, 0, y, 127, y);
            ssd1306_show(&disp);
            sleep_ms(SLEEPTIME);
            ssd1306_clear(&disp);
        }

        for(size_t font_i=0; font_i<sizeof(fonts)/sizeof(fonts[0]); ++font_i) {
            uint8_t c=32;
            while(c<=126) {
                uint8_t i=0;
                for(; i<num_chars_per_disp[font_i]; ++i) {
                    if(c>126)
                        break;
                    buf[i]=c++;
                }
                buf[i]=0;

                ssd1306_draw_string_with_font(&disp, 8, 24, 2, fonts[font_i], buf);
                ssd1306_show(&disp);
                sleep_ms(800);
                ssd1306_clear(&disp);
            }
        }

        ssd1306_bmp_show_image(&disp, image_data, image_size);
        ssd1306_show(&disp);
        sleep_ms(2000);
    }
}
