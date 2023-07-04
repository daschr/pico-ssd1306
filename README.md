[![example vid](ssd1306-example.gif)]()

# pico-ssd1306
Simple library for using ssd1306 displays with the Raspberry Pi Pico and the pico-sdk.

## Usage
* copy `font.h`, `ssd1306.c` and `ssd1306.h` to your project 
* see example

## Documentation
* online [documentation](https://daschr.github.io/pico-ssd1306/)
* see `ssd1306.h` and example

## Example
* set `PICO_SDK_PATH`
* goto `example`
* `mkdir build && cd build && cmake .. && make`
* copy the `ssd1306-example.uf2` to your Pico

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
