/* 
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "pico/stdlib.h"
#include "bsp/board.h"
#include "tusb.h"
#include "hardware/i2c.h"
#include "ssd1306.h"
#include "ssd1306_widgets.h"
#include "usb_descriptors.h"
#include "basic_font.h"

#define TXT_BUFFER_SIZE 1024
#define ITF 0

// I2C defines
#define I2C_ID i2c1
#define I2C_BAUD 200000
#define I2C_SDA_PIN 2
#define I2C_SCL_PIN 3

// Display Defines
#define DISPLAY_SIZE_X 128
#define DISPLAY_SIZE_Y 64
#define FONT_SIZE_X 8
#define FONT_SIZE_Y 8
#define DCHAR_BUF_SIZE 8 * 25
#define DISPLAY_I2C_ADDR 0x3C
#define SLEEPTIME 500

CREATE_DISPLAY(DISPLAY_SIZE_X, DISPLAY_SIZE_Y, I2C_ID, DISPLAY_I2C_ADDR, 0, 0, main_display);

CREATE_TEXT_BOX_WIDGET(DISPLAY_SIZE_X, DISPLAY_SIZE_Y, 0, 0, basic_font, main_text_box);

void display_hw_init() {
  gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
  gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);
  gpio_pull_up(I2C_SCL_PIN);
  gpio_pull_up(I2C_SDA_PIN);
  i2c_init(I2C_ID, I2C_BAUD);
}

void display_init() {
  text_buffer_clear(&main_text_box.tb);
  ssd1306_init(&main_display);
  ssd1306_clear(&main_display);
  ssd1306_show(&main_display);
}

//--------------------------------------------------------------------+
// MACRO CONSTANT TYPEDEF PROTYPES
//--------------------------------------------------------------------+

/* Blink pattern
 * - 250 ms  : device not mounted
 * - 1000 ms : device mounted
 * - 2500 ms : device is suspended
 */
enum  {
  BLINK_NOT_MOUNTED = 250,
  BLINK_MOUNTED = 1000,
  BLINK_SUSPENDED = 2500,
};


static uint32_t blink_interval_ms = BLINK_NOT_MOUNTED;

void led_blinking_task(void);
void cdc_task();
void display_task();

/*------------- MAIN -------------*/
int main(void)
{
  board_init();
  display_hw_init();
  tusb_init();
  display_init();
  

  while (1)
  {
    tud_task(); // tinyusb device task
    cdc_task();
    led_blinking_task();
    display_task();
  }
}

//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+

// Invoked when device is mounted
void tud_mount_cb(void)
{
  blink_interval_ms = BLINK_MOUNTED;
}

// Invoked when device is unmounted
void tud_umount_cb(void)
{
  blink_interval_ms = BLINK_NOT_MOUNTED;
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en)
{
  (void) remote_wakeup_en;
  blink_interval_ms = BLINK_SUSPENDED;
}

// Invoked when usb bus is resumed
void tud_resume_cb(void)
{
  blink_interval_ms = BLINK_MOUNTED;
}

// USB CDC
void cdc_task() {

  static char display_text_buffer[TXT_BUFFER_SIZE];
  // connected() check for DTR bit
  // Most but not all terminal client set this when making connection
  // if ( tud_cdc_n_connected(itf) )
  {
    if (tud_cdc_n_available(ITF)) {
      uint8_t input_buf[64];

      uint32_t count = tud_cdc_n_read(ITF, input_buf, sizeof(input_buf));

      // echo back to both serial ports
      for (uint32_t i = 0; i < count; i++) {
        snprintf(display_text_buffer, TXT_BUFFER_SIZE, "0x%2X ", input_buf[i]);
        tud_cdc_n_write_str(ITF, display_text_buffer);
        if (input_buf[i] == 13 || input_buf[i] == 10) {
          tud_cdc_n_write_char(ITF, 13);
          tud_cdc_n_write_char(ITF, 10);
        }
        process_char(&main_text_box, input_buf[i]);
      }
      tud_cdc_n_write_flush(ITF);
    }
  }
}

void display_task() {
  if (main_text_box.updated) {
    ssd1306_clear(&main_display);
    render_textbox(&main_display, &main_text_box);
    ssd1306_show(&main_display);
  }
}

void led_blinking_task(void)
{
  static uint32_t start_ms = 0;
  static bool led_state = false;

  // blink is disabled
  if (!blink_interval_ms) return;

  // Blink every interval ms
  if ( board_millis() - start_ms < blink_interval_ms) return; // not enough time
  start_ms += blink_interval_ms;

  board_led_write(led_state);
  led_state = 1 - led_state; // toggle
}
