/*#ifndef DHT22_H
#define DHT22_H

#include "pico/stdlib.h"

typedef enum {
    DHT22_OK,
    DHT22_ERROR_TIMEOUT,
    DHT22_ERROR_CHECKSUM
} dht22_status_t;

dht22_status_t dht22_read(uint gpio, float *temperature, float *humidity);

#endif

*/
#ifndef DHT22_H
#define DHT22_H

#include <stdint.h>

// Resultados possíveis para a leitura
#define DHT22_OK 0
#define DHT22_ERROR_CHECKSUM -1
#define DHT22_ERROR_TIMEOUT -2

// Função para ler temperatura e umidade do DHT22
int dht22_read(uint32_t pin, float *temperature, float *humidity);


#endif // DHT22_H
