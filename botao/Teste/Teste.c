/**
 * main_button.c
 * 
 * Teste isolado da biblioteca de botões para a BitDogLab.
 * - LED azul: GPIO 12
 * - Botão A: BITDOGLAB_BUTTON_A_PIN (definido em button.h, ex.: GPIO 5)
 * - Botão B: BITDOGLAB_BUTTON_B_PIN (definido em button.h, ex.: GPIO 6)
 *
 * O Botão A alterna o LED e o Botão B imprime uma mensagem.
 */

 #include "pico/stdlib.h"
 #include "button.h"
 #include <stdio.h>
 
 #define LED_BLUE 12
 #define BUTTON_A_PIN BITDOGLAB_BUTTON_A_PIN  // ex.: 5
 #define BUTTON_B_PIN BITDOGLAB_BUTTON_B_PIN  // ex.: 6
 
 volatile bool led_state = false;
 
 // Callback para o Botão A: alterna o LED azul.
 void button_a_callback(button_t *btn) {
     led_state = !led_state;
     gpio_put(LED_BLUE, led_state);
     printf("Botão A (pino %d) acionado. LED %s.\n", btn->pin, led_state ? "aceso" : "apagado");
 }
 
 // Callback para o Botão B: apenas imprime mensagem.
 void button_b_callback(button_t *btn) {
     printf("Botão B (pino %d) acionado.\n", btn->pin);
 }
 
 int main(void) {
     stdio_init_all();
 
     // Configuração do LED azul como saída
     gpio_init(LED_BLUE);
     gpio_set_dir(LED_BLUE, GPIO_OUT);
     gpio_put(LED_BLUE, false);
 
     // Cria as instâncias dos botões A e B utilizando a biblioteca.
     button_t *botaoA = create_button(BUTTON_A_PIN, button_a_callback);
     button_t *botaoB = create_button(BUTTON_B_PIN, button_b_callback);
 
     // Loop principal. Como o tratamento ocorre via interrupção, apenas mantenha o loop ativo.
     while (true) {
         tight_loop_contents();
     }
     return 0;
 }
 