#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/dma.h"
#include "microphone_dma.h"

static uint dma_channel;
static dma_channel_config dma_cfg;
static uint16_t adc_buffer[SAMPLES];

void microphone_init(void) {
    adc_gpio_init(MIC_PIN);
    adc_init();
    adc_select_input(MIC_CHANNEL);

    adc_fifo_setup(
        true,   // Habilita FIFO
        true,   // Habilita request de DMA
        1,      // Threshold para DMA é 1 amostra
        false,  // Não usa bit de erro
        false   // Mantém 12 bits
    );
    adc_set_clkdiv(ADC_CLOCK_DIV);

    dma_channel = dma_claim_unused_channel(true);
    dma_cfg = dma_channel_get_default_config(dma_channel);
    channel_config_set_transfer_data_size(&dma_cfg, DMA_SIZE_16);
    channel_config_set_read_increment(&dma_cfg, false);
    channel_config_set_write_increment(&dma_cfg, true);
    channel_config_set_dreq(&dma_cfg, DREQ_ADC);
}

void sample_mic(void) {
    adc_fifo_drain();
    adc_run(false);

    dma_channel_configure(dma_channel, &dma_cfg,
        adc_buffer,
        &adc_hw->fifo,
        SAMPLES,
        true
    );

    adc_run(true);
    dma_channel_wait_for_finish_blocking(dma_channel);
    adc_run(false);
}

float mic_power(void) {
    float sum = 0.f;
    for (uint i = 0; i < SAMPLES; ++i)
        sum += adc_buffer[i] * adc_buffer[i];
    sum /= SAMPLES;
    return sqrt(sum);
}

uint8_t get_intensity(float v) {
    uint8_t count = 0;
    while ((v -= ADC_STEP/20) > 0.f)
        ++count;
    return count;
}
