[![example vid](ssd1306-example.gif)]()

# pico-ssd1306

Simple library for using SSD1306 OLED displays with the Raspberry Pi Pico and the pico-sdk.

**This fork supports both SPI and I2C interfaces** - you can switch between them using a compile-time macro.

## Features

- Support for both **SPI** and **I2C** communication protocols
- Compile-time protocol selection (no runtime overhead)
- Drawing primitives: pixels, lines, rectangles
- Text rendering with built-in and custom fonts
- BMP image display support
- Multiple display sizes (128x64, 128x32, 64x32, etc.)

## Usage

1. Copy `font.h`, `ssd1306.c` and `ssd1306.h` to your project
2. Select your protocol (see below)
3. See the example for implementation details

## Protocol Selection

### Method 1: Edit the header file

In `ssd1306.h`, uncomment the protocol you want to use:

```c
// Uncomment ONE of the following:
// #define SSD1306_USE_I2C
#define SSD1306_USE_SPI    // Default
```

### Method 2: Via CMake (recommended)

```bash
# For SPI (default):
cmake .. -DSSD1306_USE_SPI=ON

# For I2C:
cmake .. -DSSD1306_USE_SPI=OFF
```

### Method 3: Define before including

```c
#define SSD1306_USE_I2C   // or SSD1306_USE_SPI
#include "ssd1306.h"
```

## Pin Configuration

### SPI Mode

| Pin | Function | Default GPIO |
|-----|----------|--------------|
| SCK | SPI Clock | 18 |
| MOSI | SPI Data | 19 |
| DC | Data/Command | 20 |
| RST | Reset | 21 |
| CS | Chip Select | 17 (or tie to GND) |

### I2C Mode

| Pin | Function | Default GPIO |
|-----|----------|--------------|
| SDA | I2C Data | 2 |
| SCL | I2C Clock | 3 |
| Address | - | 0x3C |

## Initialization

### SPI Mode

```c
#define SSD1306_USE_SPI
#include "ssd1306.h"

// Initialize SPI
spi_init(spi0, 1000000);
gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);

// Initialize display
ssd1306_t disp;
disp.external_vcc = false;
ssd1306_init(&disp, 128, 64, PIN_DC, PIN_RST, spi0);
```

### I2C Mode

```c
#define SSD1306_USE_I2C
#include "ssd1306.h"

// Initialize I2C
i2c_init(i2c1, 400000);
gpio_set_function(PIN_SDA, GPIO_FUNC_I2C);
gpio_set_function(PIN_SCL, GPIO_FUNC_I2C);
gpio_pull_up(PIN_SDA);
gpio_pull_up(PIN_SCL);

// Initialize display
ssd1306_t disp;
disp.external_vcc = false;
ssd1306_init(&disp, 128, 64, 0x3C, i2c1);
```

## Documentation

* Original [documentation](https://daschr.github.io/pico-ssd1306/)
* See `ssd1306.h` and the example

## Example

* Set `PICO_SDK_PATH` environment variable
* Go to `example` directory
* Build with SPI (default):
  ```bash
  mkdir build && cd build
  cmake ..
  make
  ```
* Build with I2C:
  ```bash
  mkdir build && cd build
  cmake .. -DSSD1306_USE_SPI=OFF
  make
  ```
* Copy the `ssd1306-example.uf2` to your Pico

## Draw Images
The library can draw monochrome bitmaps using the functions [*ssd1306_bmp_show_image*](https://daschr.github.io/pico-ssd1306/ssd1306_8h.html#a89d1f4edb34d5860df01a62512cc3949) and [*ssd1306_bmp_show_image_with_offset*](https://daschr.github.io/pico-ssd1306/ssd1306_8h.html#a1624a5ea20392d5614b84094e94160b0).

For converting an image to the supported format, you can do the following:

* install [ImageMagick](https://imagemagick.org/)
* use `convert you_image.png -monochrome your_image.bmp`

For embedding your image, you can use tools like *bin2c* (included in the *hxtools* package on Debian).

Alternatively, you can you the version included in this repo:

* go in the *tools/* directory
* `make`
* usage: `./bin2c your_image.bmp your_image.h`

You may also take a look at the example in the *example/* directory.

## Fonts

You can also use or own fonts when drawing with *ssd1306_draw_char_with_font* or *ssd1306_draw_string_with_font*.
The format is:
 1. a 1-D uint8_t array
 2. structure of the array:
    1. the first element of the array is the *height*
    2. the second element of the array is the *width*
    3. the third element of the array is the *additional spacing between chars*
    4. the fourth element of the array is the *the first ascii character, this array stores*
    5. the fifth element of the array is the *the last ascii character, this array stores*
    6. the following elements encode the pixels of the characters vertically line by line ([see](https://jared.geek.nz/2014/jan/custom-fonts-for-microcontrollers#drawing-fonts)); a line can be encoded as more than one `uint8_t` values, when the *height* is greater than 8;
please look at `font.h` and the fonts in the `example/` directory
