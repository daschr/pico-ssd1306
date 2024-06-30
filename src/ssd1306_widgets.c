#include "ssd1306_widgets.h"

static void render_char_to_screen(ssd1306_t *disp, const font *f, char c, uint16_t pos_x, uint16_t pos_y) {
    const char *sprite_start = &(f->bitmap_buffer[(c - f->first_char_in_font) * f->bytes_per_char]);
    ssd1306_blit(disp, sprite_start, f->char_height, f->char_width, pos_x, pos_y);
}

// this assumes that the text box is at least the size of a single character
void render_textbox(ssd1306_t *disp, ssd1306_textbox *tbw) {
    uint16_t lines_in_textbox = tbw->height/tbw->fnt->char_height  ;
    uint16_t columns_in_textbox = tbw->width/tbw->fnt->char_width  ; 
    uint16_t tbox_col = 0;
    uint16_t tbox_line = 0;
    // this guarantees that there
    size_t txt_idx = get_start_of_nth_line_from_end(&tbw->tb, lines_in_textbox);
    while (txt_idx < tbw->tb.filled) {
        char c = tbw->tb.buffer[(tbw->tb.start + txt_idx) % TEXT_BUF_LEN];
        txt_idx ++;
        if ( c == '\n' || c == 13) {
            tbox_col=0;
            if (tbox_line < lines_in_textbox) {
               tbox_line++; 
            }
        } else {
            if (tbox_col < columns_in_textbox) {
                render_char_to_screen(disp, tbw->fnt, c, tbw->pos_x + (tbox_col * tbw->fnt->char_width), tbw->pos_y + (tbox_line * tbw->fnt->char_height));
                tbox_col ++;
            }
        }
    }
    tbw->updated = false;
}

// have the text widget process a character
void process_char(ssd1306_textbox *text_widget, char c) {
    text_buffer_process_char(&text_widget->tb, c);
    text_widget->updated = true;
}

void clear_textbox(ssd1306_textbox *text_widget) {
    text_buffer_clear(&text_widget->tb);
    text_widget->updated = true;
}
