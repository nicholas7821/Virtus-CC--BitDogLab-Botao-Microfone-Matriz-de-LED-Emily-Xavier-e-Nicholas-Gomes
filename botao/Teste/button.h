/**
 * @file button.h
 * @brief Biblioteca de debounce para botões na Raspberry Pi Pico adaptada para a BitDogLab
 *
 * Nesta versão, os botões são mapeados conforme o pinout da BitDogLab:
 *   - Botão A: BITDOGLAB_BUTTON_A_PIN (ex.: GPIO 5)
 *   - Botão B: BITDOGLAB_BUTTON_B_PIN (ex.: GPIO 6)
 *
 * Baseada no fork de https://github.com/jkroso/pico-button.c.
 *

#ifndef PICO_BUTTON_H
#define PICO_BUTTON_H

#include "pico/stdlib.h"

#ifdef __cplusplus
extern "C" {
#endif
*/

#define DEBOUNCE_US 200

/* Mapeamento dos botões para a BitDogLab */
#define BITDOGLAB_BUTTON_A_PIN 5
#define BITDOGLAB_BUTTON_B_PIN 6

typedef struct button_t {
    uint8_t pin;
    bool state;
    void (*onchange)(struct button_t *button);
} button_t;

typedef void (*handler)(void *argument);

typedef struct {
    void *argument;
    handler fn;
} closure_t;

long long int handle_button_alarm(long int a, void *p);
void handle_button_interrupt(void *p);
void handle_interrupt(uint gpio, uint32_t events);
void listen(uint pin, int condition, handler fn, void *arg);
button_t * create_button(int pin, void (*onchange)(button_t *));

#ifdef __cplusplus
}
#endif

//#endif // PICO_BUTTON_H
