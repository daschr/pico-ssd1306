add_library(ssd1306)
target_sources(ssd1306
        PRIVATE 
        ${CMAKE_CURRENT_LIST_DIR}/ssd1306.c
        )
target_include_directories(ssd1306 PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/../fonts
        )
target_compile_definitions(ssd1306
        PUBLIC
        SSD1306_USE_DMA)
target_link_libraries(ssd1306
        PRIVATE
        pico_stdlib
        )
target_link_libraries(ssd1306
        PUBLIC
        hardware_i2c
        hardware_dma
        )


add_library(ssd1306_widgets)
target_sources(ssd1306_widgets
        PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/ssd1306_widgets.c
        ${CMAKE_CURRENT_LIST_DIR}/text_buffer.c
        )
target_include_directories(ssd1306_widgets PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/../fonts
        )
target_link_libraries(ssd1306_widgets
        PUBLIC
        ssd1306
        )
