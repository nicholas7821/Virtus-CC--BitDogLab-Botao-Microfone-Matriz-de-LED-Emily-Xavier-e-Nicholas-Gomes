#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/dma.h"
#include "neopixel.c"

// ------------------ Definições de Microfone ------------------
#define MIC_CHANNEL     2
#define MIC_PIN         (26 + MIC_CHANNEL)
#define ADC_CLOCK_DIV   96.f
#define SAMPLES         200
#define ADC_ADJUST(x)   ((x) * 3.3f / (1 << 12u) - 1.65f) // Converte a leitura RMS para tensão, centrada em 1.65V
#define ADC_MAX         3.3f
#define ADC_STEP        (3.3f / 5.f)

// ------------------ Definições de LED ------------------
#define LED_PIN         7
#define LED_COUNT       25

// ------------------ Globais de DMA e Buffer ------------------
static uint dma_channel;
static dma_channel_config dma_cfg;
static uint16_t adc_buffer[SAMPLES];

// ------------------ Prototipagem de Funções ------------------
void microphone_init(void);
void sample_mic(void);
float mic_power(void);
uint8_t get_intensity(float v);

// ------------------ main ------------------
int main(void) {
    // Inicializa a saída serial para debug
    stdio_init_all();

    // Aguarda alguns segundos para permitir abrir o monitor serial
    sleep_ms(5000);

    // Inicializa a matriz de LEDs
    printf("Inicializando Neopixel...\n");
    npInit(LED_PIN, LED_COUNT);

    // Inicializa o ADC e DMA para o microfone
    printf("Configurando ADC e DMA para microfone...\n");
    microphone_init();
    printf("Configuracao concluida.\n");

    // Teste rápido de amostragem
    sample_mic();
    printf("Iniciando loop principal...\n");

    // Variável para suavização (Exponential Moving Average)
    static float smoothed_mag = 0.0f;
    float alpha = 0.2f;  // Fator de suavização (0 < alpha <= 1)

    while (true) {
        // 1) Amostra o microfone via DMA
        sample_mic();

        // 2) Calcula a potência média (RMS)
        float raw_rms = mic_power();

        // 3) Converte esse valor RMS para tensão (ajustada) e magnitude
        float adjusted = ADC_ADJUST(raw_rms);
        float magnitude = 2.f * fabsf(adjusted);  // Multiplica por 2 e toma valor absoluto

        // 4) Suaviza usando Exponential Moving Average
        smoothed_mag = alpha * magnitude + (1.0f - alpha) * smoothed_mag;

        // 5) Converte o valor suavizado em intensidade (0 a 4)
        uint8_t intensity = get_intensity(smoothed_mag);

        // Limpa a matriz de LEDs
        npClear();

        // Implementa uma "barra de progresso" linear com base na intensidade
        int ledsToLight = 0;
        // Ajusta quantos LEDs acender para cada intensidade
        switch (intensity) {
            case 0: ledsToLight = 0;  break;  // Nenhum LED
            case 1: ledsToLight = 6;  break;  // ~1/4 dos 25 LEDs
            case 2: ledsToLight = 12; break;  // ~metade
            case 3: ledsToLight = 18; break;  
            case 4: ledsToLight = 25; break;  // Todos
        }

        // Define cor para cada intensidade (exemplo)
        uint8_t r = 0, g = 0, b = 0;
        switch (intensity) {
            case 1: g = 80;  break;               // Verde
            case 2: r = 80; g = 80;  break;       // Amarelo
            case 3: r = 120; g = 60;  break;      // Laranja
            case 4: r = 120; g = 0;   break;      // Vermelho
        }

        // Acende os primeiros "ledsToLight" LEDs
        for (int i = 0; i < ledsToLight && i < LED_COUNT; i++) {
            npSetLED(i, r, g, b);
        }

        // Atualiza a matriz de LEDs
        npWrite();

        // Imprime dados de debug no serial
        printf("RMS: %.4f, Adj: %.4f, Magn: %.4f, Smooth: %.4f, Int: %u, LEDs: %d\n",
               raw_rms, adjusted, magnitude, smoothed_mag, intensity, ledsToLight);

        // Pequeno atraso antes da próxima leitura
        sleep_ms(200);
    }

    return 0;
}

// ------------------ Implementação das Funções ------------------

/**
 * @brief Inicializa o ADC e configura o DMA para leitura do microfone.
 */
void microphone_init(void) {
    // Configura o pino do microfone para ADC
    adc_gpio_init(MIC_PIN);
    // Inicializa o ADC
    adc_init();
    // Seleciona o canal
    adc_select_input(MIC_CHANNEL);

    // Configura o FIFO do ADC para disparar DMA a cada leitura
    adc_fifo_setup(
        true,   // Habilita FIFO
        true,   // Habilita request de DMA
        1,      // Threshold: 1 amostra
        false,  // Não usa bit de erro
        false   // Mantém 12 bits
    );
    adc_set_clkdiv(ADC_CLOCK_DIV);

    // Configura o canal DMA
    dma_channel = dma_claim_unused_channel(true);
    dma_cfg = dma_channel_get_default_config(dma_channel);
    channel_config_set_transfer_data_size(&dma_cfg, DMA_SIZE_16);
    channel_config_set_read_increment(&dma_cfg, false);
    channel_config_set_write_increment(&dma_cfg, true);
    channel_config_set_dreq(&dma_cfg, DREQ_ADC);
}

/**
 * @brief Realiza uma amostragem do microfone, transferindo SAMPLES para adc_buffer via DMA.
 */
void sample_mic() {
    // Limpa o FIFO e desliga ADC para configurar DMA
    adc_fifo_drain();
    adc_run(false);

    dma_channel_configure(
        dma_channel,
        &dma_cfg,
        adc_buffer,        // Buffer de destino
        &adc_hw->fifo,     // Fonte: FIFO do ADC
        SAMPLES,           // Número de amostras
        true               // Inicia DMA
    );

    // Liga o ADC e aguarda a transferência
    adc_run(true);
    dma_channel_wait_for_finish_blocking(dma_channel);
    adc_run(false);
}

/**
 * @brief Calcula o valor RMS (potência média) das amostras no buffer.
 */
float mic_power() {
    float sum = 0.f;
    for (uint i = 0; i < SAMPLES; i++) {
        sum += adc_buffer[i] * adc_buffer[i];
    }
    sum /= SAMPLES;
    return sqrtf(sum);
}

/**
 * @brief Mapeia o valor (já ajustado) para uma intensidade de 0 a 4.
 */
uint8_t get_intensity(float v) {
    uint8_t count = 0;
    // Diminui v em passos de (ADC_STEP / 20)
    // Ajuste se quiser mais ou menos sensibilidade
    while ((v -= ADC_STEP/20) > 0.f) {
        ++count;
    }
    // count não pode exceder 4
    if (count > 4) count = 4;
    return count;
}
