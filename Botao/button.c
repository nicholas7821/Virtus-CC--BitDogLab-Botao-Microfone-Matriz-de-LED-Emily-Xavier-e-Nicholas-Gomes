/**
 * Embarcatech 
 * Exemplo Botão com a BitDogLab
 */

#include "pico/stdlib.h"

#define LED_BLUE 12   // GPIO conectado ao terminal azul do LED RGB
#define BUTTON_A 5    // GPIO conectado ao Botão A
#define BUTTON_B 6    // GPIO conectado ao Botão B
int main() {
    // Configuração do GPIO do LED como saída
    stdio_init_all();  // Inicializa a saída serial para debug (printf)
    
    gpio_init(LED_BLUE);
    gpio_set_dir(LED_BLUE, GPIO_OUT);
    gpio_put(LED_BLUE, false);  // Inicialmente, o LED está apagado

    // Configuração do GPIO do Botão A como entrada com pull-up interno
    gpio_init(BUTTON_A);
    gpio_set_dir(BUTTON_A, GPIO_IN);
    gpio_pull_up(BUTTON_A);

    // Configuração do Botão B como entrada com pull-up interno:
    gpio_init(BUTTON_B);
    gpio_set_dir(BUTTON_B, GPIO_IN);
    gpio_pull_up(BUTTON_B);
     
    bool led_state = false;  // Variável para manter o estado do LED
     

    while (true) {
        // Lê o estado do Botão A
        bool button_a_state = gpio_get(BUTTON_A);  // HIGH = solto, LOW = pressionado
        bool button_b_state = gpio_get(BUTTON_B);
        // Atualiza o estado do LED com base no estado do Botão A
                // Se o Botão A for pressionado (estado LOW), alterna o LED.
        if (!button_a_state) {  
            gpio_put(LED_BLUE,true );  // Se solto (HIGH), LED acende; se pressionado (LOW), apaga
            printf("Botão A pressionado!\n");
            sleep_ms(300);
         }
         if (!button_b_state) {
            gpio_put(LED_BLUE, false);
            printf("Botão B pressionado!\n");
            sleep_ms(300);
        }
         // Pequeno delay para evitar leituras inconsistentes (debounce simples)
        sleep_ms(50);
    }
    return 0;
}


