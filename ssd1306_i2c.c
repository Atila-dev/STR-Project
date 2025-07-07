#include "ssd1306_i2c.h"
#include "hardware/i2c.h"
#include <string.h>
#include <ctype.h>

static uint8_t ssd1306_buffer[SSD1306_WIDTH * SSD1306_HEIGHT / 8];

static void ssd1306_command(uint8_t cmd) {
    uint8_t buf[2] = {0x80, cmd};
    i2c_write_blocking(SSD1306_I2C_PORT, SSD1306_I2C_ADDR, buf, 2, false);
}

void ssd1306_init(void) {
    i2c_init(SSD1306_I2C_PORT, 400 * 1000);
    gpio_set_function(SSD1306_I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(SSD1306_I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(SSD1306_I2C_SDA);
    gpio_pull_up(SSD1306_I2C_SCL);
    sleep_ms(250);

    static const uint8_t cmds[] = {
        0xAE, 0x20, 0x00, 0xB0, 0xC8, 0x00, 0x10, 0x40, 0x81, 0xFF,
        0xA1, 0xA6, 0xA8, 0x3F, 0xA4, 0xD3, 0x00, 0xD5, 0xF0, 0xD9,
        0x22, 0xDA, 0x12, 0xDB, 0x20, 0x8D, 0x14, 0xAF
    };
    for (size_t i = 0; i < sizeof(cmds); ++i) {
        ssd1306_command(cmds[i]);
    }
    ssd1306_clear();
    ssd1306_update();
}

void ssd1306_clear(void) {
    memset(ssd1306_buffer, 0, sizeof(ssd1306_buffer));
}

void ssd1306_update(void) {
    for (uint8_t page = 0; page < 8; page++) {
        ssd1306_command(0xB0 | page);
        ssd1306_command(0x00);
        ssd1306_command(0x10);
        uint8_t buf[SSD1306_WIDTH + 1];
        buf[0] = 0x40;
        memcpy(&buf[1], &ssd1306_buffer[SSD1306_WIDTH * page], SSD1306_WIDTH);
        i2c_write_blocking(SSD1306_I2C_PORT, SSD1306_I2C_ADDR, buf, SSD1306_WIDTH + 1, false);
    }
}

// Exemplo de fonte 8x8 (substitua pelo seu array real)
extern const uint8_t font5x7[][5];

void ssd1306_draw_char(int16_t x, int16_t y, char c) {
    if (c < 32 || c > 126) c = '?';
    for (int i = 0; i < 5; i++) {
        ssd1306_buffer[x + (y/8)*SSD1306_WIDTH + i] = font5x7[c - 32][i];
    }
    ssd1306_buffer[x + (y/8)*SSD1306_WIDTH + 5] = 0x00; // espaço entre caracteres
}

void ssd1306_draw_string(int16_t x, int16_t y, const char *str) {
    while (*str) {
        ssd1306_draw_char(x, y, *str++);
        x += 6; // 5 pixels da fonte + 1 espaço
        if (x > SSD1306_WIDTH - 6) break;
    }
}