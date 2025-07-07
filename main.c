#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "ssd1306.h"
#include <stdio.h>
#include "hardware/pwm.h"
#include "hardware/clocks.h"
#include "hardware/adc.h"
#include "dht22.h"
#include <stdio.h>
#include <string.h>

// filepath: c:\Users\atila\TESTE2\TESTE2.c

// Definições dos pinos e sensores
#define I2C_PORT i2c0
#define SDA_PIN 14  
#define SCL_PIN 15  
#define DHT_PIN 18
#define MQ2_ADC_PIN 20

// Pinos dos LEDs de alerta
#define LED_VERDE 4   
#define LED_AMARELO 9  
#define LED_VERMELHO 8  

// Configuração do buzzer
#define BUZZER_PIN 21
#define BUZZER_FREQUENCY 2500
#define BUTTON_PIN 5
#define CALIB_BUTTON_PIN 6

void gerar_json(float temperatura, float umidade, uint16_t gas, uint16_t gas_amb, const char* status, char* buffer, size_t buffer_size) {
    snprintf(buffer, buffer_size,
        "{"
            "\"temperatura\":%.1f,"
            "\"umidade\":%.1f,"
            "\"gas\":%u,"
            "\"gas_amb\":%u,"
            "\"status\":\"%s\""
        "}",
        temperatura, umidade, gas, gas_amb, status
    );
}

void setup_leds() {
    gpio_init(LED_VERDE);
    gpio_set_dir(LED_VERDE, GPIO_OUT);
    gpio_init(LED_AMARELO);
    gpio_set_dir(LED_AMARELO, GPIO_OUT);
    gpio_init(LED_VERMELHO);
    gpio_set_dir(LED_VERMELHO, GPIO_OUT);
}

void set_leds(bool verde, bool amarelo, bool vermelho) {
    gpio_put(LED_VERDE, verde);
    gpio_put(LED_AMARELO, amarelo);
    gpio_put(LED_VERMELHO, vermelho);
}

void pwm_init_buzzer(uint pin) {
    gpio_set_function(pin, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(pin);
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, clock_get_hz(clk_sys) / (BUZZER_FREQUENCY * 4096));
    pwm_init(slice_num, &config, true);
    pwm_set_gpio_level(pin, 0);
}

void beep_alert(uint pin, int type) {
    uint slice_num = pwm_gpio_to_slice_num(pin);
    uint freq = (type == 1) ? 2000 : 3000;  
    pwm_set_clkdiv(slice_num, clock_get_hz(clk_sys) / (freq * 4096));
    int beeps = (type == 1) ? 1 : 3;
    int beep_time = (type == 1) ? 200 : 100;  
    for (int i = 0; i < beeps; i++) {
        pwm_set_gpio_level(pin, 2048);
        sleep_ms(beep_time);
        pwm_set_gpio_level(pin, 0);
        sleep_ms(beep_time);
    }
}
uint16_t read_mq2_adc() {
    adc_select_input(0); // ADC0 = GPIO26
    return adc_read();   // Retorna valor de 0 a 4095
}

int main() {

    stdio_init_all();

    // Inicialização do I2C e display
    i2c_init(i2c1, 400 * 1000); // Use i2c1, igual ao driver
    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(SDA_PIN);
    gpio_pull_up(SCL_PIN);

    sleep_ms(100); // Pequeno delay para garantir que o I2C estabilize

    ssd1306_init();

    adc_init();
    gpio_init(DHT_PIN);
    gpio_set_dir(DHT_PIN, GPIO_IN);

    setup_leds();
    gpio_init(BUZZER_PIN);
    gpio_set_dir(BUZZER_PIN, GPIO_OUT);
    pwm_init_buzzer(BUZZER_PIN);
    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_PIN); // Botão entre pino e GND

    gpio_init(CALIB_BUTTON_PIN);
    gpio_set_dir(CALIB_BUTTON_PIN, GPIO_IN);
    gpio_pull_up(CALIB_BUTTON_PIN); // Botão entre pino e GND

    uint16_t mq2_ambient = 0;
    bool last_calib_button = 1;
    
    float temperature = 0.0f;
    float humidity = 0.0f;
    bool alarm_enabled = true;
    bool last_button_state = 1;

    while (1) {
    bool calib_button = gpio_get(CALIB_BUTTON_PIN);
        uint16_t mq2_value = read_mq2_adc();

        if (!calib_button && last_calib_button) { // Pressionado
            mq2_ambient = mq2_value;
        }
        last_calib_button = calib_button;

        // --- Alerta de gás ---
        bool gas_alert = (mq2_value > mq2_ambient + 50); // 50 é margem, ajuste conforme necessário

            // Leitura do botão (BitDogLab: detecta borda de descida)
    bool button_state = gpio_get(BUTTON_PIN);
    if (!button_state && last_button_state) { // Pressionado
        alarm_enabled = !alarm_enabled;
    }
    last_button_state = button_state;

        if (dht22_read(DHT_PIN, &temperature, &humidity) != 0) {
        ssd1306_clear();
        ssd1306_draw_string(10, 10, "Erro ao ler sensor");
        ssd1306_update();
        sleep_ms(2000);
        continue;
    }

    char status_str[20];
    if (gas_alert) {
            snprintf(status_str, sizeof(status_str), "GAS PERIGO!");
            if (alarm_enabled) {
                set_leds(0, 0, 1);
                beep_alert(BUZZER_PIN, 2);
            } else {
                set_leds(0, 0, 0);
                pwm_set_gpio_level(BUZZER_PIN, 0);
            }
        } else if (temperature >= 36.0 || temperature <= 23.0 || humidity > 80.0 || humidity < 30.0) {
            snprintf(status_str, sizeof(status_str), "PERIGO!");
            if (alarm_enabled) {
                set_leds(0, 0, 1);
                beep_alert(BUZZER_PIN, 2);
            } else {
                set_leds(0, 0, 0);
                pwm_set_gpio_level(BUZZER_PIN, 0);
            }
        } else if ((temperature >= 32.0 && temperature < 36.0) ||
                   (humidity >= 60.0 && humidity <= 80.0)) {
            snprintf(status_str, sizeof(status_str), "ATENCAO");
            if (alarm_enabled) {
                set_leds(0, 1, 0);
                beep_alert(BUZZER_PIN, 1);
            } else {
                set_leds(0, 0, 0);
                pwm_set_gpio_level(BUZZER_PIN, 0);
            }
        } else {
            snprintf(status_str, sizeof(status_str), "SAUDAVEL");
            set_leds(1, 0, 0);
            pwm_set_gpio_level(BUZZER_PIN, 0);
        }

        char temp_str[20], hum_str[20], gas_str[32];
        snprintf(temp_str, sizeof(temp_str), "Temp: %.1fC", temperature);
        snprintf(hum_str, sizeof(hum_str), "Umid: %.1f%%", humidity);
        snprintf(gas_str, sizeof(gas_str), "Gas: %u (Amb: %u)", mq2_value, mq2_ambient);

        ssd1306_clear();
        ssd1306_draw_string(0, 0, "Monitoramento");
        ssd1306_draw_string(0, 16, hum_str);
        ssd1306_draw_string(0, 32, temp_str);
        ssd1306_draw_string(0, 48, status_str);
        ssd1306_draw_string(0, 56, gas_str);
        ssd1306_update();

        sleep_ms(2000);

    char json_buffer[128];
    gerar_json(temperature, humidity, mq2_value, mq2_ambient, status_str, json_buffer, sizeof(json_buffer));

    // Exemplo: envie para serial, web, etc.
    printf("%s\n", json_buffer);
    }
    return 0;
}