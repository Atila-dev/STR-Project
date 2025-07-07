#ifndef SSD1306_I2C_H
#define SSD1306_I2C_H

#include <stdint.h>
#include "pico/stdlib.h"

#define SSD1306_I2C_PORT i2c1
#define SSD1306_I2C_SDA 14
#define SSD1306_I2C_SCL 15
#define SSD1306_I2C_ADDR 0x3C

#define SSD1306_WIDTH 128
#define SSD1306_HEIGHT 64

void ssd1306_init(void);
void ssd1306_clear(void);
void ssd1306_update(void);
void ssd1306_draw_string(int16_t x, int16_t y, const char *string);

#endif