cmake_minimum_required(VERSION 3.13)

# Pico SDK
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

# Auto-name project/target after the folder name
get_filename_component(PROJ_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
project(${PROJ_NAME} C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

set(TARGET_NAME ${PROJ_NAME})

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(${TARGET_NAME}
    main.c
    i2c/i2c_bus.c
    i2c/sensors/aht20/aht20.c
    i2c/sensors/veml7700/veml7700.c
    i2c/ssd1306/ssd1306.c
    oled_text/font_table.c
    encoder/pec11r.c
)

target_link_libraries(${TARGET_NAME} 
    pico_stdlib 
    pico_cyw43_arch_none
    hardware_i2c
)

target_include_directories(${TARGET_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/i2c
    ${CMAKE_CURRENT_LIST_DIR}/i2c/sensors/aht20
    ${CMAKE_CURRENT_LIST_DIR}/i2c/sensors/veml7700
    ${CMAKE_CURRENT_LIST_DIR}/i2c/ssd1306
    ${CMAKE_CURRENT_LIST_DIR}/oled_text
    ${CMAKE_CURRENT_LIST_DIR}/encoder
)

pico_enable_stdio_usb(${TARGET_NAME} 1)
pico_enable_stdio_uart(${TARGET_NAME} 0)

pico_add_extra_outputs(${TARGET_NAME})