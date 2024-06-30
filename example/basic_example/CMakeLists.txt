cmake_minimum_required(VERSION 3.13)

# initialize the SDK based on PICO_SDK_PATH
# note: this must happen before project()
include(pico_sdk_import.cmake)

project(ssd1306-examples)

set(CMAKE_C_STANDARD 11)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
option(USE_DMA_FOR_DISPLAY "Use the DMA engine to move the display buffer to the display" OFF)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# rest of your project
add_subdirectory(../../src lib)
add_executable(ssd1306-example
	example.c
)

target_include_directories(ssd1306-example
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/../../fonts
)

if(USE_DMA_FOR_DISPLAY)
  message(STATUS "Using DMA")
  target_compile_definitions(ssd1306-example
    PUBLIC
    SSD1306_USE_DMA
  )
  target_link_libraries(ssd1306-example
    hardware_dma
  )
endif(USE_DMA_FOR_DISPLAY)

target_link_libraries(ssd1306-example
  pico_stdlib
  hardware_i2c
  ssd1306
  ssd1306_widgets
  )

pico_enable_stdio_usb(ssd1306-example 0) 
pico_enable_stdio_uart(ssd1306-example 1) 
# create map/bin/hex/uf2 file in addition to ELF.
pico_add_extra_outputs(ssd1306-example)

