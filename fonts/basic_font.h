#ifndef _basic_font_h
#define _basic_font_h
#include "font.h"
#include "font_struct.h"

const font basic_font = {
	.bytes_per_char = 5,
	.char_width = 5,
	.first_char_in_font = 32,
	.char_height = 8,
	.bitmap_buffer = (char *)&font_8x5[5],
};
#endif
