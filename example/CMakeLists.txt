cmake_minimum_required(VERSION 3.13)

# initialize the SDK based on PICO_SDK_PATH
# note: this must happen before project()
include(pico_sdk_import.cmake)

project(ssd1306-example)

set(CMAKE_C_STANDARD 11)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# ============================================================================
# Protocol selection - set to ON for SPI, OFF for I2C
# ============================================================================
option(SSD1306_USE_SPI "Use SPI interface instead of I2C" ON)

# rest of your project

add_executable(ssd1306-example
	example.c ../ssd1306.c
)

target_include_directories(ssd1306-example
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/../
)

# Link appropriate hardware library based on protocol selection
if(SSD1306_USE_SPI)
    message(STATUS "Building with SPI interface")
    target_compile_definitions(ssd1306-example PRIVATE SSD1306_USE_SPI)
    target_link_libraries(ssd1306-example pico_stdlib hardware_spi)
else()
    message(STATUS "Building with I2C interface")
    target_compile_definitions(ssd1306-example PRIVATE SSD1306_USE_I2C)
    target_link_libraries(ssd1306-example pico_stdlib hardware_i2c)
endif()

pico_enable_stdio_usb(ssd1306-example 1) 
pico_enable_stdio_uart(ssd1306-example 0) 

# create map/bin/hex/uf2 file in addition to ELF.
pico_add_extra_outputs(ssd1306-example)

