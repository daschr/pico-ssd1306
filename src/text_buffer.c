#include "text_buffer.h"

void text_buffer_clear(text_buffer* tbuf) {
  tbuf->filled = 0;
  tbuf->start = 0;
};

// get the start of the line that index idx is located in
// this function assumes that idx is not larger than TEXT_BUF_LEN
size_t get_beginning_of_line(text_buffer *tb, size_t idx) {
  size_t i = idx;
  for (; i > 0; i--) {
      if (tb->buffer[(tb->start+i) % TEXT_BUF_LEN] == '\n') {
          return i-1;
      }
  }
  return i;
}

void append_char(text_buffer *tb, char c) {
  size_t write_idx = (tb->start + tb->filled) % TEXT_BUF_LEN;
  tb->buffer[write_idx] = c;
  if (tb->filled == TEXT_BUF_LEN) {
      tb->start ++;
  } else {
      tb->filled ++;
  }
}

void text_buffer_process_char(text_buffer *tb, char c) {
  if (c == '\b' && tb->filled > 0) { // backspace
      tb->filled --; 
      return;
  }
  if ((c >= 32 && c <= 126) || c == '\r') {
      append_char(tb, c);
      return;
  }
  if ( c == 9 ) {
      append_char(tb, ' ');
      append_char(tb, ' ');
      append_char(tb, ' ');
      append_char(tb, ' ');
  }
}

// find the index of the char that is the start char for the nth line back (if it exists)
// if there are less than n lines, then it returns the start of the text buffer.
// In either way it will be the top left character in a text box
size_t get_start_of_nth_line_from_end(text_buffer *tb, uint16_t lines_to_display) {
  uint16_t lines = 0;
  size_t current_buf_idx = tb->filled;
  while (lines<lines_to_display && current_buf_idx > 0) {
      current_buf_idx = get_beginning_of_line(tb, current_buf_idx);
      lines_to_display ++;
  }
  return current_buf_idx;
}

