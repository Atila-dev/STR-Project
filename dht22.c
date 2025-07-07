#include "dht22.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"

// Constantes de temporização para o protocolo do DHT22
#define DHT22_START_SIGNAL_DELAY 18000 // 18ms em microssegundos
#define DHT22_RESPONSE_WAIT_TIMEOUT 100 // Timeout para resposta em microssegundos

// Função auxiliar para esperar até o pino mudar de estado
static inline int wait_for_pin_state(uint32_t pin, bool state, uint32_t timeout_us) {
    uint32_t start = time_us_32();
    while (gpio_get(pin) != state) {
        if ((time_us_32() - start) > timeout_us) {
            return -1; // Timeout
        }
    }
    return 0; // Sucesso
}

// Função principal para ler dados do sensor DHT22
int dht22_read(uint32_t pin, float *temperature, float *humidity) {
    uint8_t data[5] = {0}; // Array para armazenar os 40 bits recebidos

    // Configura o pino como saída e envia o sinal de início
    gpio_set_dir(pin, GPIO_OUT);
    gpio_put(pin, 0); // Sinal baixo
    sleep_us(DHT22_START_SIGNAL_DELAY); // Aguarda o tempo de sinal
    gpio_put(pin, 1); // Sinal alto
    sleep_us(30); // Pausa de 30us antes de mudar para entrada

    // Configura o pino como entrada para receber a resposta do sensor
    gpio_set_dir(pin, GPIO_IN);

    // Aguarda o sensor responder
    if (wait_for_pin_state(pin, 0, DHT22_RESPONSE_WAIT_TIMEOUT) != 0) return DHT22_ERROR_TIMEOUT;
    if (wait_for_pin_state(pin, 1, DHT22_RESPONSE_WAIT_TIMEOUT) != 0) return DHT22_ERROR_TIMEOUT;
    if (wait_for_pin_state(pin, 0, DHT22_RESPONSE_WAIT_TIMEOUT) != 0) return DHT22_ERROR_TIMEOUT;

    // Lê os 40 bits (5 bytes) de dados do sensor
    for (int i = 0; i < 40; i++) {
        // Aguarda o início do pulso
        if (wait_for_pin_state(pin, 1, DHT22_RESPONSE_WAIT_TIMEOUT) != 0) return DHT22_ERROR_TIMEOUT;

        // Mede a duração do pulso
        uint32_t pulse_start = time_us_32();
        if (wait_for_pin_state(pin, 0, DHT22_RESPONSE_WAIT_TIMEOUT) != 0) return DHT22_ERROR_TIMEOUT;

        uint32_t pulse_length = time_us_32() - pulse_start;

        // Cada bit é determinado pela duração do pulso (bit 1 > 40us)
        if (pulse_length > 40) {
            data[i / 8] |= (1 << (7 - (i % 8))); // Define o bit
        }
    }

    // Calcula o checksum e verifica os dados
    uint8_t checksum = data[0] + data[1] + data[2] + data[3];
    if (checksum != data[4]) {
        return DHT22_ERROR_CHECKSUM; // Checksum inválido
    }

    // Converte os dados para temperatura e umidade
    *humidity = ((data[0] << 8) | data[1]) * 0.1; // Umidade em %
    *temperature = ((data[2] & 0x7F) << 8 | data[3]) * 0.1; // Temperatura em °C
    if (data[2] & 0x80) {
        *temperature *= -1; // Se o bit de sinal estiver setado, a temperatura é negativa
    }

    return DHT22_OK; // Sucesso
}
