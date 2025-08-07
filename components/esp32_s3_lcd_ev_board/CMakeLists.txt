file(GLOB_RECURSE SRCS src/*.c)

idf_component_register(
    SRCS ${SRCS}
    INCLUDE_DIRS "include" "include/bsp" "../lvgl" "../esp_lvgl_port/include"
    PRIV_INCLUDE_DIRS "priv_include"
    REQUIRES driver esp_lcd esp_adc
    PRIV_REQUIRES esp_timer spiffs esp_psram
)
