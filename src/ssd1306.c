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

#include <hardware/i2c.h>
#include <pico/binary_info.h>
#include <string.h>
#include <stdio.h>
#include <pico/stdlib.h>

#include "ssd1306.h"
#ifdef SSD1306_USE_DMA
#include "hardware/dma.h"
#endif
#include "font_struct.h"

inline static void swap(int32_t *a, int32_t *b) {
    int32_t *t=a;
    *a=*b;
    *b=*t;
}

#ifndef SSD1306_USE_DMA
inline static void fancy_write(i2c_inst_t *i2c, uint8_t addr, const uint8_t *src, size_t len, char *name) {
    switch(i2c_write_blocking(i2c, addr, src, len, true)) {
    case PICO_ERROR_GENERIC:
        printf("[%s] addr not acknowledged!\n", name);
        break;
    case PICO_ERROR_TIMEOUT:
        printf("[%s] timeout!\n", name);
        break;
    default:
        //printf("[%s] wrote successfully %lu bytes!\n", name, len);
        break;
    }
}
#endif

#ifdef SSD1306_USE_DMA
inline static void ssd1306_write(ssd1306_t *p, uint8_t val) {
    uint8_t d[2]= {0x00, val};
    i2c_write_blocking(p->i2c_i, p->address, d, 2, true);
}
#else
inline static void ssd1306_write(ssd1306_t *p, uint8_t val) {
    uint8_t d[2]= {0x00, val};
    fancy_write(p->i2c_i, p->address, d, 2, "ssd1306_write");
}
#endif

#ifdef SSD1306_USE_DMA
bool ssd1306_init(ssd1306_t *p) {
    uint8_t startup_commands[]= {
        SET_DISP,
        SET_DISP_CLK_DIV,
        0x80,
        SET_MUX_RATIO,
        p->height - 1,
        SET_DISP_OFFSET,
        0x00,
        SET_DISP_START_LINE,
        SET_CHARGE_PUMP, \
        p->external_vcc ? 0x10 : 0x14,
        SET_SEG_REMAP | 0x01,
        SET_COM_OUT_DIR | 0x08,
        SET_COM_PIN_CFG,
        p->width > 2 * p->height ? 0x02 : 0x12,
        SET_CONTRAST,
        0xff,
        SET_PRECHARGE,
        p->external_vcc ? 0x22 : 0xF1,
        SET_VCOM_DESEL,
        0x30,
        SET_ENTIRE_ON,
        SET_NORM_INV,
        SET_DISP | 0x01,
        SET_MEM_ADDR,
        0x00,
    };
    dma_channel_claim(p->dma_channel);
    for(size_t i=0; i<sizeof(startup_commands); ++i) {
        ssd1306_write(p, startup_commands[i]);
    }
}
#else
bool ssd1306_init(ssd1306_t *p, uint16_t width, uint16_t height, uint8_t address, i2c_inst_t *i2c_instance) {
    p->width=width;
    p->height=height;
    p->pages=height/8;
    p->address=address;

    p->i2c_i=i2c_instance;


    p->bufsize=(p->pages)*(p->width);
    if((p->buffer=malloc(p->bufsize+1))==NULL) {
        p->bufsize=0;
        return false;
    }

    ++(p->buffer);

    // from https://github.com/makerportal/rpi-pico-ssd1306
    uint8_t cmds[]= {
        SET_DISP,
        // timing and driving scheme
        SET_DISP_CLK_DIV,
        0x80,
        SET_MUX_RATIO,
        height - 1,
        SET_DISP_OFFSET,
        0x00,
        // resolution and layout
        SET_DISP_START_LINE,
        // charge pump
        SET_CHARGE_PUMP,
        p->external_vcc?0x10:0x14,
        SET_SEG_REMAP | 0x01,           // column addr 127 mapped to SEG0
        SET_COM_OUT_DIR | 0x08,         // scan from COM[N] to COM0
        SET_COM_PIN_CFG,
        width>2*height?0x02:0x12,
        // display
        SET_CONTRAST,
        0xff,
        SET_PRECHARGE,
        p->external_vcc?0x22:0xF1,
        SET_VCOM_DESEL,
        0x30,                           // or 0x40?
        SET_ENTIRE_ON,                  // output follows RAM contents
        SET_NORM_INV,                   // not inverted
        SET_DISP | 0x01,
        // address setting
        SET_MEM_ADDR,
        0x00,  // horizontal
    };

    for(size_t i=0; i<sizeof(cmds); ++i)
        ssd1306_write(p, cmds[i]);

    return true;
}
#endif

#ifndef SSD1306_USE_DMA
inline void ssd1306_deinit(ssd1306_t *p) {
    free(p->buffer-1);
}
#endif

inline void ssd1306_poweroff(ssd1306_t *p) {
    ssd1306_write(p, SET_DISP|0x00);
}

inline void ssd1306_poweron(ssd1306_t *p) {
    ssd1306_write(p, SET_DISP|0x01);
}

inline void ssd1306_contrast(ssd1306_t *p, uint8_t val) {
    ssd1306_write(p, SET_CONTRAST);
    ssd1306_write(p, val);
}

inline void ssd1306_invert(ssd1306_t *p, uint8_t inv) {
    ssd1306_write(p, SET_NORM_INV | (inv & 1));
}

inline void ssd1306_clear(ssd1306_t *p) {
    memset(p->buffer, 0, p->bufsize);
}

// assumes the bytes are organized in horizontal bars (called pages) 8 pixels high (as one byte per column of the bar and then the subsequent
// byte in the memmory fills  occupies the next column but all the same rows as the previous byte)
// The sprite is assumed to be in the same paged layout with a minimum of 8 bits height (padded if needed).
void ssd1306_blit(ssd1306_t *disp, const char *sprite, uint32_t sprite_height, uint32_t sprite_width, uint32_t start_col, uint32_t start_row) {
    uint8_t top_page = start_row >> 3;
    uint8_t top_row_in_page = start_row % 8;
    uint8_t sprite_dangling_bits = sprite_height % 8;
    uint8_t full_sprite_pages = (sprite_height >> 3);
    // handle the full rows in the sprite
    for (int i = 0; i < full_sprite_pages; i ++) {
        uint32_t displ_buffer_addr = (top_page + i) * disp->width + start_col;
        for (int j = 0; j < sprite_width; j++) {
            uint32_t sprite_addr = i * sprite_width + j;
            if (top_row_in_page > 0) {
                disp->buffer[displ_buffer_addr+j] &= ~(0xff << top_row_in_page);
                disp->buffer[displ_buffer_addr+j] |= sprite[sprite_addr] << top_row_in_page;
                disp->buffer[displ_buffer_addr+j+disp->width] &= ~(0xff >> (8 - top_row_in_page));
                disp->buffer[displ_buffer_addr+j+disp->width] |= sprite[sprite_addr] >> ( 8 - top_row_in_page);
            } else {
                disp->buffer[displ_buffer_addr+j] = sprite[sprite_addr];
            }
        }
    }
    // handle a possibly dangling row
    if (sprite_dangling_bits > 0) {
        uint32_t displ_buffer_start_addr = (top_page + full_sprite_pages) * disp->width + start_col;
        uint32_t sprite_start_addr = full_sprite_pages * sprite_width;
        if (sprite_dangling_bits > (8 - top_row_in_page)) {
            for (int i = 0; i<sprite_width; i++) {
               disp->buffer[displ_buffer_start_addr+i] &= ~(0xff >> (8-top_row_in_page));
               disp->buffer[displ_buffer_start_addr+i] |= sprite[sprite_start_addr+i] & (0xff >> (8-top_row_in_page));
               disp->buffer[displ_buffer_start_addr+i+disp->width] &= ~(0xff >> (sprite_dangling_bits -  8 + top_row_in_page));
               disp->buffer[displ_buffer_start_addr+i+disp->width] |= sprite[sprite_start_addr+i+sprite_width] >> (sprite_dangling_bits -  8 + top_row_in_page);
            }
        } else {
            for (int i = 0; i<sprite_width; i++) {
               disp->buffer[displ_buffer_start_addr+i] &= ~(0xff >> (8-sprite_dangling_bits));
               disp->buffer[displ_buffer_start_addr+i] |= sprite[sprite_start_addr+i] & (0xff >> (8-sprite_dangling_bits));
            }
        }
    }

}


// Blit a sprite onto the screen the packing of the sprite is considered tp be
// loose, such that the memmory occupied by a display column is considered padded to the next byte 
// assuming one bit is a single pixel so a 5 pixel tall sprite would use one byte per
// column and a 13 bit tall sprite would use 2 bytes per column.
// The content of the padded bits are irrelevant
// it also assumes that the display buffer include pad bits at the end of every column (if needed)
// It is also assumed that the sprite fits entirely onto the screen. This will fail if the sprite
// partly exits the screen area
//
// sprite -> Pointer to the buffer containing the sprite in column major order
// sprite_height -> the number of rows covered by a sprite
// sprite_width -> the number of columns covered by a sprite
// start_col -> column containing the top left pixel of the sprite
// start_row -> row containing the top left pixel of the column
void blit_row_mayor_display_buffer(ssd1306_t *disp, const uint8_t* sprite, uint32_t sprite_height, uint32_t sprite_width, uint32_t start_col, uint32_t start_row) {
    uint32_t dbuf_bytes_per_line = (disp->width >> 3) + (disp->width % 8 > 0 ? 1 : 0); // length of a column in the display in bytes with padding
    uint8_t dbuf_start_bit = start_col % 8; // the first bit in the byte that the sprite would modify
    uint8_t sprite_line_pad_byte = sprite_width % 8 > 0 ? 1 : 0; // determin if the sprite has a byte that includes padding
    uint8_t sprite_bytes_per_line = (sprite_width >> 3) + sprite_line_pad_byte; // length of a sprite column in byte including padding
    // now we blit the columns of the sprite into the appropriate bytes of the display buffer
    for (uint32_t line=0;line<sprite_height;line++) {
        uint32_t dbuf_line_start_addr = dbuf_bytes_per_line * (start_row + line) + (start_col >> 3); // determin the first byte of the display buffer we are blitting into
        uint32_t sprite_line_start_addr = line * sprite_bytes_per_line; // deterime the start byte in the sprite memory
        // now for every non padded byte in a sprite column we copy it to the display buffer
        for (uint32_t line_byte=0; line_byte<(sprite_width>>3);line_byte++) {
            if (dbuf_start_bit > 0) {
                // if we do not have a byte alligned write into the display buffer memory we need some extra bit shifting
                disp->buffer[dbuf_line_start_addr+line_byte]   &= ~(0xff << dbuf_start_bit);
                disp->buffer[dbuf_line_start_addr+line_byte]   |= sprite[sprite_line_start_addr+line_byte] << dbuf_start_bit;
                disp->buffer[dbuf_line_start_addr+line_byte+1] &= ~(0xff >> (8 - dbuf_start_bit));
                disp->buffer[dbuf_line_start_addr+line_byte+1] |= sprite[sprite_line_start_addr+line_byte] >> (8 - dbuf_start_bit);
            } else {
                // if we are aligned we are fine
                disp->buffer[dbuf_line_start_addr+line_byte] = sprite[sprite_line_start_addr+line_byte];
            }
        }
        if (sprite_line_pad_byte) {
            // the last byte is padded, so now we need to deal with a padded and a non padded byte
            uint8_t display_buffer_byte = disp->buffer[dbuf_line_start_addr+sprite_bytes_per_line-1];
            uint8_t sprite_buffer_byte = sprite[sprite_line_start_addr + sprite_bytes_per_line - 1];
            uint8_t sprite_bits = sprite_width % 8; // how many bits still need to be written into the display buffer
            uint8_t dbuf_byte_unmodified_bits = 8 - dbuf_start_bit; // how many bits are still 'untouched' in the current buffer byte 
            // so we now need to figure out if the bytes at the end of our sprite line
            // fit into the bits that are left in the display buffer byte
            if (sprite_bits > dbuf_byte_unmodified_bits) {
                //if we need to write more bits into the display buffer that fit into the current byte
                //first take care of the bits that need to go into the current display buffer byte
                display_buffer_byte &= ~(0xff << dbuf_start_bit);
                display_buffer_byte |= sprite_buffer_byte << dbuf_start_bit;
                disp->buffer[dbuf_line_start_addr+sprite_bytes_per_line-1] = display_buffer_byte;
               
                // update the variables that we can now treat the easy case
                display_buffer_byte = disp->buffer[dbuf_line_start_addr+sprite_bytes_per_line];
                sprite_bits -= dbuf_byte_unmodified_bits;
                sprite_buffer_byte = sprite_buffer_byte >> dbuf_byte_unmodified_bits;
                dbuf_byte_unmodified_bits = 8;
            }
            display_buffer_byte &=  ~((0xff >> (8 - sprite_bits)) << dbuf_start_bit); // mask the bits that will be written to by the sprite
            display_buffer_byte |= (sprite_buffer_byte & (0xff >> (8 - sprite_bits))) << dbuf_start_bit; // write the sprite bits into the dbuffer
            disp->buffer[dbuf_line_start_addr+sprite_bytes_per_line-1] = display_buffer_byte; // write back the display buffer
        }
    }
}

void ssd1306_clear_pixel(ssd1306_t *p, uint32_t x, uint32_t y) {
    if(x>=p->width || y>=p->height) return;

    p->buffer[x+p->width*(y>>3)]&=~(0x1<<(y&0x07));
}

void ssd1306_draw_pixel(ssd1306_t *p, uint32_t x, uint32_t y) {
    if(x>=p->width || y>=p->height) return;

    p->buffer[x+p->width*(y>>3)]|=0x1<<(y&0x07); // y>>3==y/8 && y&0x7==y%8
}

void ssd1306_draw_line(ssd1306_t *p, int32_t x1, int32_t y1, int32_t x2, int32_t y2) {
    if(x1>x2) {
        swap(&x1, &x2);
        swap(&y1, &y2);
    }

    if(x1==x2) {
        if(y1>y2)
            swap(&y1, &y2);
        for(int32_t i=y1; i<=y2; ++i)
            ssd1306_draw_pixel(p, x1, i);
        return;
    }

    float m=(float) (y2-y1) / (float) (x2-x1);

    for(int32_t i=x1; i<=x2; ++i) {
        float y=m*(float) (i-x1)+(float) y1;
        ssd1306_draw_pixel(p, i, (uint32_t) y);
    }
}

void ssd1306_draw_square(ssd1306_t *p, uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
    for(uint32_t i=0; i<width; ++i)
        for(uint32_t j=0; j<height; ++j)
            ssd1306_draw_pixel(p, x+i, y+j);

}

void ssd1306_draw_empty_square(ssd1306_t *p, uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
    ssd1306_draw_line(p, x, y, x+width, y);
    ssd1306_draw_line(p, x, y+height, x+width, y+height);
    ssd1306_draw_line(p, x, y, x, y+height);
    ssd1306_draw_line(p, x+width, y, x+width, y+height);
}

void ssd1306_draw_char_with_font(ssd1306_t *p, uint32_t x, uint32_t y, uint32_t scale, const uint8_t *font, char c) {
    if(c<font[3]||c>font[4])
        return;

    uint32_t parts_per_line=(font[0]>>3)+((font[0]&7)>0);
    for(uint8_t w=0; w<font[1]; ++w) { // width
        uint32_t pp=(c-font[3])*font[1]*parts_per_line+w*parts_per_line+5;
        for(uint32_t lp=0; lp<parts_per_line; ++lp) {
            uint8_t line=font[pp];

            for(int8_t j=0; j<8; ++j, line>>=1) {
                if(line & 1)
                    ssd1306_draw_square(p, x+w*scale, y+((lp<<3)+j)*scale, scale, scale);
            }

            ++pp;
        }
    }
}

void ssd1306_draw_string_with_font(ssd1306_t *p, uint32_t x, uint32_t y, uint32_t scale, const uint8_t *font, const char *s) {
    for(int32_t x_n=x; *s; x_n+=(font[1]+font[2])*scale) {
        ssd1306_draw_char_with_font(p, x_n, y, scale, font, *(s++));
    }
}

static inline uint32_t ssd1306_bmp_get_val(const uint8_t *data, const size_t offset, uint8_t size) {
    switch(size) {
    case 1:
        return data[offset];
    case 2:
        return data[offset]|(data[offset+1]<<8);
    case 4:
        return data[offset]|(data[offset+1]<<8)|(data[offset+2]<<16)|(data[offset+3]<<24);
    default:
        __builtin_unreachable();
    }
    __builtin_unreachable();
}

void ssd1306_bmp_show_image_with_offset(ssd1306_t *p, const uint8_t *data, const long size, uint32_t x_offset, uint32_t y_offset) {
    if(size<54) // data smaller than header
        return;

    const uint32_t bfOffBits=ssd1306_bmp_get_val(data, 10, 4);
    const uint32_t biSize=ssd1306_bmp_get_val(data, 14, 4);
    const int32_t biWidth=(int32_t) ssd1306_bmp_get_val(data, 18, 4);
    const int32_t biHeight=(int32_t) ssd1306_bmp_get_val(data, 22, 4);
    const uint16_t biBitCount=(uint16_t) ssd1306_bmp_get_val(data, 28, 2);
    const uint32_t biCompression=ssd1306_bmp_get_val(data, 30, 4);

    if(biBitCount!=1) // image not monochrome
        return;

    if(biCompression!=0) // image compressed
        return;

    const int table_start=14+biSize;
    uint8_t color_val;

    for(uint8_t i=0; i<2; ++i) {
        if(!((data[table_start+i*4]<<16)|(data[table_start+i*4+1]<<8)|data[table_start+i*4+2])) {
            color_val=i;
            break;
        }
    }

    uint32_t bytes_per_line=(biWidth/8)+(biWidth&7?1:0);
    if(bytes_per_line&3)
        bytes_per_line=(bytes_per_line^(bytes_per_line&3))+4;

    const uint8_t *img_data=data+bfOffBits;

    int step=biHeight>0?-1:1;
    int border=biHeight>0?-1:biHeight;

    for(uint32_t y=biHeight>0?biHeight-1:0; y!=border; y+=step) {
        for(uint32_t x=0; x<biWidth; ++x) {
            if(((img_data[x>>3]>>(7-(x&7)))&1)==color_val)
                ssd1306_draw_pixel(p, x_offset+x, y_offset+y);
        }
        img_data+=bytes_per_line;
    }
}

void ssd1306_bmp_show_image(ssd1306_t *p, const uint8_t *data, const long size) {
    ssd1306_bmp_show_image_with_offset(p, data, size, 0, 0);
}

#ifdef SSD1306_USE_DMA
void copy_to_dma_tx(ssd1306_t *disp) {
    disp->dma_tx_buffer[0] = 1u << I2C_IC_DATA_CMD_RESTART_LSB | 0x0040;
    for (int i = 0; i < disp->bufsize-1; i++) {
        disp->dma_tx_buffer[i+1] =  disp->buffer[i];
    }
    disp->dma_tx_buffer[disp->bufsize] = (1u << I2C_IC_DATA_CMD_STOP_LSB) | disp->buffer[disp->bufsize-1];
}

void print_content_of_dma_tx(ssd1306_t *disp, void (*sprint_fn)(char *)) {
    char print_buf[10];
    for (int i = 0; i < disp->height/8; i++) {
        for(int j = 0; j < disp->width; j++) {
            sprintf(print_buf, "%04x", disp->dma_tx_buffer[i*disp->width + j + 1]);
            sprint_fn(print_buf);
        }
    }
}

// we need to load all the data from out actual display buffer into
// into the buffer that will be read out by the DMA. Then we need to set
// the appropriate control bits for the top 8 bits of every 16 bit word
void print_out_buffer_content(ssd1306_t *disp, void (*sprint_fn)(char *)) {
    unsigned int pages = disp->height / 8;
    char text_out[10];
    for (int i = 0; i < pages; i++) {
        sprint_fn("\r\n");
        for (int j = 0; j < disp->width; j ++) {
            sprintf(text_out, "%02x", disp->buffer[i*pages + j]);
            sprint_fn(text_out);
        }
    }
}

void ssd1306_show(ssd1306_t *p) {
    // if there is already a transfer running, wait until it has completed
    dma_channel_wait_for_finish_blocking(p->dma_channel);
    copy_to_dma_tx(p);
    // now set the address of the display that we want to write to
    p->i2c_i->hw->enable = 0;
    p->i2c_i->hw->tar = p->address;
    p->i2c_i->hw->enable = 1;
    uint8_t payload[]= {SET_COL_ADDR, 0, p->width-1, SET_PAGE_ADDR, 0, p->pages-1};
    if(p->width==64) {
        payload[1]+=32;
        payload[2]+=32;
    }

    for(size_t i=0; i<sizeof(payload); ++i)
        ssd1306_write(p, payload[i]);

    
    // the things that are left to do is to set the read address, and set the transfer count
    // (we are doing 16bit transfers that means the count equals the amount of bytes in the
    // diplay buffer
    dma_channel_config dma_config = dma_channel_get_default_config(p->dma_channel);
    channel_config_set_transfer_data_size(&dma_config, DMA_SIZE_16);
    channel_config_set_read_increment(&dma_config, true);
    channel_config_set_write_increment(&dma_config, false);
    // configure the i2c channel to cooperate with the dma engine
    channel_config_set_dreq(&dma_config, i2c_get_dreq(p->i2c_i, true));
    dma_channel_configure(
        p->dma_channel,                        // Channel to be configured
        &dma_config,                           // The configuration we just created
        &i2c_get_hw(p->i2c_i)->data_cmd,       // The initial write address
        p->dma_tx_buffer,                      // The initial read address
        p->bufsize+1,                          // Number of transfers; in this case each is 2 byte.
        true                                   // Start immediately.
    );
}
#else
void ssd1306_show(ssd1306_t *p) {
    uint8_t payload[]= {SET_COL_ADDR, 0, p->width-1, SET_PAGE_ADDR, 0, p->pages-1};
    if(p->width==64) {
        payload[1]+=32;
        payload[2]+=32;
    }

    for(size_t i=0; i<sizeof(payload); ++i)
        ssd1306_write(p, payload[i]);

    *(p->buffer-1)=0x40;
    fancy_write(p->i2c_i, p->address, p->buffer-1, p->bufsize+1, "ssd1306_show");
}
#endif
