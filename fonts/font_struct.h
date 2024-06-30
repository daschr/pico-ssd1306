#ifndef _font_struct_h
#define _font_struct_h
#include <stdint.h>
/**
 * Struct that holds a font and all relevant information
 */
typedef struct {
    uint16_t bytes_per_char; // the size of the char sprite in bytes (including padding)
    uint16_t char_width; // width of the caracter in pixels (how many columns the sprite spans)
    char first_char_in_font; // the first char that is included in the font
    uint16_t char_height; // height of the character in pixels (how many rows the sprite spans)
    const char *bitmap_buffer;
} font;
#endif
