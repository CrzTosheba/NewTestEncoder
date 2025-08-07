# For more information about build system see
# https://docs.espressif.com/projects/esp-idf/en/latest/api-guides/build-system.html
# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
add_compile_options("-Wno-attributes") # For LVGL code


# Add common application components
set(EXTRA_COMPONENT_DIRS 
    ${CMAKE_CURRENT_LIST_DIR}/components/cmake_utilities
    ${CMAKE_CURRENT_LIST_DIR}/components/esp32_s3_lcd_ev_board
    ${CMAKE_CURRENT_LIST_DIR}/components/esp_lvgl_port
    ${CMAKE_CURRENT_LIST_DIR}/components/lvgl
)

set(COMPONENTS main) # "Trim" the build. Include the minimal set of components; main and anything it depends on.

project(display_lvgl_demos)

