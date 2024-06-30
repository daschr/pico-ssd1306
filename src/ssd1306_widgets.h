#include "ssd1306.h"
#include "text_buffer.h"
#include "font_struct.h"

// Create an instance of a Text Box
#define CREATE_TEXT_BOX_WIDGET(width_, height_, pos_x_, pos_y_, font_struct_name_, varname_) \
    ssd1306_textbox varname_ = {\
	.pos_x = pos_x_,\
	.pos_y = pos_y_,\
	.width = width_,\
	.height = height_,\
	.fnt = &font_struct_name_\
    };\

typedef struct {
	uint16_t pos_x;
	uint16_t pos_y;
	uint16_t width;
	uint16_t height;
	const font *fnt;
	text_buffer tb;
	bool updated;
} ssd1306_textbox;

// Render a text box to a screen
void render_textbox(ssd1306_t *disp, ssd1306_textbox *text_widget);

// translate a character in a text box
void process_char(ssd1306_textbox *text_widget, char c);

// clear the content of the text box
void clear_textbox(ssd1306_textbox *text_widget);
