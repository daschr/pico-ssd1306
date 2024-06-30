#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

#include "ssd1306.h"
#include "image.h"
#include "acme_5_outlines_font.h"
#include "bubblesstandard_font.h"
#include "crackers_font.h"
#include "BMSPA_font.h"
#include "font.h"

const uint8_t num_chars_per_disp[]={7,7,7,5};
const uint8_t *fonts[] = {acme_font, bubblesstandard_font, crackers_font, BMSPA_font};

#define SLEEPTIME 30
#ifdef SSD1306_USE_DMA
CREATE_DISPLAY(128, 64, i2c1, 0x3C, 0, 0, main_display) ;
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
    i2c_init(i2c1, 400000);
    gpio_set_function(2, GPIO_FUNC_I2C);
    gpio_set_function(3, GPIO_FUNC_I2C);
    gpio_pull_up(2);
    gpio_pull_up(3);
}

void ssd1306_draw_string(ssd1306_t *disp, uint32_t x, uint32_t y, uint32_t scale, const char *s) { 
    ssd1306_draw_string_with_font(disp, x, y, scale, font_8x5, s);
}


void animation(void) {
    const char *words[]= {"SSD1306", "DISPLAY", "DRIVER"};

#ifdef SSD1306_USE_DMA
    ssd1306_init(&main_display);        
#else
    ssd1306_t display_01;
    display_01.external_vcc=false;
    ssd1306_init(&display_01, 128, 64, 0x3C, i2c1);
#endif
    ssd1306_clear(&main_display);

    printf("ANIMATION!\n");

    char buf[8];

    for(;;) {
        for(int y=0; y<31; ++y) {
            ssd1306_draw_line(&main_display, 0, y, 127, y);
            ssd1306_show(&main_display);
            sleep_ms(SLEEPTIME);
            ssd1306_clear(&main_display);
        }

        for(int y=0, i=1; y>=0; y+=i) {
            ssd1306_draw_line(&main_display, 0, 31-y, 127, 31+y);
            ssd1306_draw_line(&main_display, 0, 31+y, 127, 31-y);
            ssd1306_show(&main_display);
            sleep_ms(SLEEPTIME);
            ssd1306_clear(&main_display);
            if(y==32) i=-1;
        }

        for(int i=0; i<sizeof(words)/sizeof(char *); ++i) {
            ssd1306_draw_string(&main_display, 8, 24, 2, words[i]);
            ssd1306_show(&main_display);
            sleep_ms(800);
            ssd1306_clear(&main_display);
        }

        for(int y=31; y<63; ++y) {
            ssd1306_draw_line(&main_display, 0, y, 127, y);
            ssd1306_show(&main_display);
            sleep_ms(SLEEPTIME);
            ssd1306_clear(&main_display);
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

                ssd1306_draw_string_with_font(&main_display, 8, 24, 2, fonts[font_i], buf);
                ssd1306_show(&main_display);
                sleep_ms(800);
                ssd1306_clear(&main_display);
            }
        }

        ssd1306_bmp_show_image(&main_display, image_data, image_size);
        ssd1306_show(&main_display);
        sleep_ms(2000);
    }
}
