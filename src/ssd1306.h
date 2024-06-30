/*
MIT License

Copyright (c) 2021 David Schramm

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

/** 
* @file ssd1306.h
* 
* simple driver for ssd1306 displays
*/

#ifndef _inc_ssd1306
#define _inc_ssd1306
#include <stdint.h>
#include "hardware/i2c.h"
#include "hardware/i2c.h"

/**
*	@brief defines commands used in ssd1306
*/
typedef enum {
    SET_CONTRAST = 0x81,
    SET_ENTIRE_ON = 0xA4,
    SET_NORM_INV = 0xA6,
    SET_DISP = 0xAE,
    SET_MEM_ADDR = 0x20,
    SET_COL_ADDR = 0x21,
    SET_PAGE_ADDR = 0x22,
    SET_DISP_START_LINE = 0x40,
    SET_SEG_REMAP = 0xA0,
    SET_MUX_RATIO = 0xA8,
    SET_COM_OUT_DIR = 0xC0,
    SET_DISP_OFFSET = 0xD3,
    SET_COM_PIN_CFG = 0xDA,
    SET_DISP_CLK_DIV = 0xD5,
    SET_PRECHARGE = 0xD9,
    SET_VCOM_DESEL = 0xDB,
    SET_CHARGE_PUMP = 0x8D
} ssd1306_command_t;

#ifdef SSD1306_USE_DMA
#include "hardware/dma.h"
/* construct and initialize the display struct used to generate the display output
 * at compile time. This allows omitting the code to define the variables at runtime
 * as all the details are known at compile time
 */
#define CREATE_DISPLAY(width_, height_, I2C, address_, dma_channel_, external_vcc_, varname_) \
    uint8_t display_buffer_ ## varname_[width_*height_/8];\
    uint16_t dma_tx_bufferbuffer_ ## varname_[(width_*height_/8)+1];\
    ssd1306_t varname_  = {\
	.dma_tx_buffer = dma_tx_bufferbuffer_ ## varname_,\
	.buffer = display_buffer_ ## varname_,\
	.bufsize = width_ * height_ / 8,\
	.width = width_,\
	.height = height_,\
	.pages = height_ / 8,\
	.address = address_,\
	.dma_channel = dma_channel_,\
	.external_vcc = external_vcc_,\
	.i2c_i = I2C\
    }
#endif

/**
*	@brief holds the configuration
*/
#ifdef SSD1306_USE_DMA
typedef struct {
    volatile uint16_t *dma_tx_buffer;
    volatile uint8_t *buffer;		/**< display buffer */
    const size_t bufsize;		/**< buffer size */
    const uint8_t width; 		/**< width of display */
    const uint8_t height;		/**< height of display */
    const uint8_t pages;		/**< stores pages of display (calculated on initialization*/
    const uint8_t address;		/**< i2c address of display*/
    const uint dma_channel;
    const uint8_t external_vcc;	/**< whether display uses external vcc */ 
    i2c_inst_t *i2c_i;		/**< i2c connection instance */
} ssd1306_t;
#else
typedef struct {
    size_t bufsize;		/**< buffer size */
    uint8_t *buffer;		/**< display buffer */
    uint8_t width; 		/**< width of display */
    uint8_t height;		/**< height of display */
    uint8_t pages;		/**< stores pages of display (calculated on initialization*/
    uint8_t address;		/**< i2c address of display*/
    i2c_inst_t *i2c_i;		/**< i2c connection instance */
    bool external_vcc;		/**< whether display uses external vcc */ 
} ssd1306_t;
#endif

#ifdef SSD1306_USE_DMA
bool ssd1306_init(ssd1306_t *p);
#else
/**
*	@brief initialize display
*
*	@param[in] p : pointer to instance of ssd1306_t
*	@param[in] width : width of display
*	@param[in] height : heigth of display
*	@param[in] address : i2c address of display
*	@param[in] i2c_instance : instance of i2c connection
*	
* 	@return bool.
*	@retval true for Success
*	@retval false if initialization failed
*/
bool ssd1306_init(ssd1306_t *p, uint16_t width, uint16_t height, uint8_t address, i2c_inst_t *i2c_instance);
#endif


/**
*	@brief deinitialize display
*
*	@param[in] p : instance of display
*
*/
void ssd1306_deinit(ssd1306_t *p);

/**
*	@brief turn off display
*
*	@param[in] p : instance of display
*
*/
void ssd1306_poweroff(ssd1306_t *p);

/**
	@brief turn on display

	@param[in] p : instance of display

*/
void ssd1306_poweron(ssd1306_t *p);
/**
	@brief Blit a sprite into the display buffer

	@param[in] disp : instance of display with display buffer
	@param[in] sprite : the buffer containing the sprite (padded if necessary)
	@param[in] sprite_height : the height of the sprite in pixels
	@param[in] sprite_width : the width of the sprite in pixels (number of columns)
	@param[in] start_col : the column on the display containing the top left corner of the sprite
	@param[in] start_row : the row containing the top left corner of the sprite
 */
void ssd1306_blit(ssd1306_t *disp, const char* sprite,
		  uint32_t sprite_height, uint32_t sprite_width,
		  uint32_t start_col, uint32_t start_row);

/**
	@brief set contrast of display

	@param[in] p : instance of display
	@param[in] val : contrast

*/
void ssd1306_contrast(ssd1306_t *p, uint8_t val);

/**
	@brief set invert display

	@param[in] p : instance of display
	@param[in] inv : inv==0: disable inverting, inv!=0: invert

*/
void ssd1306_invert(ssd1306_t *p, uint8_t inv);

/**
	@brief display buffer, should be called on change

	@param[in] p : instance of display

*/
void ssd1306_show(ssd1306_t *p);

/**
	@brief clear display buffer

	@param[in] p : instance of display

*/
void ssd1306_clear(ssd1306_t *p);

/**
	@brief clear pixel on buffer

	@param[in] p : instance of display
	@param[in] x : x position
	@param[in] y : y position
*/
void ssd1306_clear_pixel(ssd1306_t *p, uint32_t x, uint32_t y);

/**
	@brief draw pixel on buffer

	@param[in] p : instance of display
	@param[in] x : x position
	@param[in] y : y position
*/
void ssd1306_set_pixel(ssd1306_t *p, uint32_t x, uint32_t y);

/**
	@brief draw pixel on buffer

	@param[in] p : instance of display
	@param[in] x1 : x position of starting point
	@param[in] y1 : y position of starting point
	@param[in] x2 : x position of end point
	@param[in] y2 : y position of end point
*/
void ssd1306_draw_line(ssd1306_t *p, int32_t x1, int32_t y1, int32_t x2, int32_t y2);
#endif

void ssd1306_bmp_show_image(ssd1306_t *p, const uint8_t *data, const long size);

void ssd1306_draw_string_with_font(ssd1306_t *p, uint32_t x, uint32_t y, uint32_t scale, const uint8_t *font, const char *s);
