cmake_minimum_required(VERSION 3.13)

# initialize the SDK based on PICO_SDK_PATH
# note: this must happen before project()
include(pico_sdk_import.cmake)

project(usb-cdc-example)

set(CMAKE_C_STANDARD 11)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
option(USE_DMA_FOR_DISPLAY "Use the DMA engine to move the display buffer to the display" ON)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# add the subdirectory containing the ssd1306 library code
add_subdirectory(../../src lib)

# create the example target
add_executable(usb-cdc-example
	usb_ssd1306.c
	usb_cdc_descriptors.c
)

# make sure the example target sees the fonts
target_include_directories(usb-cdc-example
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/../../fonts
)

if(USE_DMA_FOR_DISPLAY)
  message(STATUS "Using DMA")
  target_compile_definitions(usb-cdc-example
    PUBLIC
    SSD1306_USE_DMA
  )
  target_link_libraries(usb-cdc-example
    hardware_dma
  )
endif(USE_DMA_FOR_DISPLAY)

target_link_libraries(usb-cdc-example
  pico_stdlib
  pico_unique_id
  hardware_i2c
  ssd1306
  ssd1306_widgets
  tinyusb_device
  tinyusb_board
 )

pico_enable_stdio_usb(usb-cdc-example 0) 
pico_enable_stdio_uart(usb-cdc-example 1) 
# create map/bin/hex/uf2 file in addition to ELF.
pico_add_extra_outputs(usb-cdc-example)
