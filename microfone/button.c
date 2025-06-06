/**
 * @file button.c
 * @brief Implementação da biblioteca de debounce para botões na Raspberry Pi Pico adaptada para BitDogLab
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "button.h"

closure_t handlers[28] = {0};
alarm_id_t alarm_ids[28] = {0};

long long int handle_button_alarm(long int a, void *p) {
    button_t *b = (button_t *)(p);
    bool state = gpio_get(b->pin);
    if (state != b->state) {
        b->state = state;
        b->onchange(b);
    }
    return 0;
}

void handle_button_interrupt(void *p) {
    button_t *b = (button_t *)(p);
    if (alarm_ids[b->pin]) cancel_alarm(alarm_ids[b->pin]);
    alarm_ids[b->pin] = add_alarm_in_us(DEBOUNCE_US, handle_button_alarm, b, true);
}

void handle_interrupt(uint gpio, uint32_t events) {
    closure_t handler = handlers[gpio];
    handler.fn(handler.argument);
}

void listen(uint pin, int condition, handler fn, void *arg) {
    gpio_set_irq_enabled_with_callback(pin, condition, true, handle_interrupt);
    closure_t *closure_instance = malloc(sizeof(closure_t));
    closure_instance->argument = arg;
    closure_instance->fn = fn;
    handlers[pin] = *closure_instance;
}

button_t * create_button(int pin, void (*onchange)(button_t *)) {
    gpio_init(pin);
    gpio_pull_up(pin);
    button_t *b = malloc(sizeof(button_t));
    listen(pin, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, handle_button_interrupt, b);
    b->pin = pin;
    b->onchange = onchange;
    b->state = gpio_get(pin);
    return b;
}
