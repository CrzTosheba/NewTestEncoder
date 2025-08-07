#ifndef _ST7701_H_
#define _ST7701_H_

#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_check.h"

#include "driver/gpio.h"
#include "bsp/esp32_s3_lcd_ev_board.h"

#define LCD_CS_Clr()    gpio_set_level(GPIO_LCD4_CS, 0)
#define LCD_CS_Set()    gpio_set_level(GPIO_LCD4_CS, 1)
#define LCD_SCK_Clr()   gpio_set_level(GPIO_LCD4_SCK, 0)
#define LCD_SCK_Set()   gpio_set_level(GPIO_LCD4_SCK, 1)
#define LCD_SDA_Clr()   gpio_set_level(GPIO_LCD4_SDA, 0)
#define LCD_SDA_Set()   gpio_set_level(GPIO_LCD4_SDA, 1)


void st7701_reg_init(void);

#endif

