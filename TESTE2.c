#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "ssd1306.h"
#include <stdio.h>
#include "hardware/pwm.h"
#include "hardware/clocks.h"
#include "hardware/adc.h"
#include "dht22.h"

// Definições dos pinos e sensores
#define I2C_PORT i2c0
#define SDA_PIN 14  
#define SCL_PIN 15  
#define DHT_PIN 18
#define MQ4_ADC_PIN 26

// Pinos dos LEDs de alerta
#define LED_VERDE 4   
#define LED_AMARELO 9  
#define LED_VERMELHO 8  

// Configuração do buzzer
#define BUZZER_PIN 21
#define BUZZER_FREQUENCY 2500

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
uint16_t read_mq4_adc() {
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

    
    float temperature = 0.0f;
    float humidity = 0.0f;

    while (1) {
        if (dht22_read(DHT_PIN, &temperature, &humidity) != 0) {
            ssd1306_clear();
            ssd1306_draw_string(10, 10, "Erro ao ler sensor");
            ssd1306_update();
            sleep_ms(2000);
            continue;
        }

        char status_str[20];
        if (temperature >= 36.0 || temperature <= 23.0 || humidity > 80.0 || humidity < 30.0) {
            snprintf(status_str, sizeof(status_str), "PERIGO!");
            set_leds(0, 0, 1);  
            beep_alert(BUZZER_PIN, 2);
        } else if ((temperature >= 32.0 && temperature < 36.0) || 
                   (humidity >= 60.0 && humidity <= 80.0)) {
            snprintf(status_str, sizeof(status_str), "ATENCAO");
            set_leds(0, 1, 0);
            beep_alert(BUZZER_PIN, 1);
        } else {
            snprintf(status_str, sizeof(status_str), "SAUDAVEL");
            set_leds(1, 0, 0);
        }
        char temp_str[20], hum_str[20];
        snprintf(temp_str, sizeof(temp_str), "Temp: %.1fC", temperature);
        snprintf(hum_str, sizeof(hum_str), "Umid: %.1f%%", humidity);


    uint16_t mq4_value = read_mq4_adc();
    char gas_str[20];
    snprintf(gas_str, sizeof(gas_str), "Gas: %u", mq4_value);

    ssd1306_clear();
    ssd1306_draw_string(0, 0, "Monitoramento");
    ssd1306_draw_string(0, 16, hum_str);
    ssd1306_draw_string(0, 32, temp_str);
    ssd1306_draw_string(0, 48, status_str);
    ssd1306_draw_string(0, 56, gas_str); // Mostra valor do MQ-4
    ssd1306_update();

        sleep_ms(2000);
    }
    return 0;
}