#include <stddef.h>
#include <stdint.h>
#define TEXT_BUF_LEN 512
enum tb_state {
    WRAP,
    TRUNC
};

typedef struct {
    char buffer[TEXT_BUF_LEN];
    size_t start;
    size_t filled;
} text_buffer;

// get the start index for the last n lines in the buffer
size_t get_start_of_nth_line_from_end(text_buffer *tb, uint16_t lines_to_display);

// clear the text buffer
void text_buffer_clear(text_buffer* tbuf);

// processes the char in such a way that only printable characters end up in the text buffer
void text_buffer_process_char(text_buffer *tb, char c);
