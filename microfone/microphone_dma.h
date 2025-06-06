#ifndef MICROPHONE_DMA_H
#define MICROPHONE_DMA_H

#include "pico/stdlib.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MIC_CHANNEL     2
#define MIC_PIN         (26 + MIC_CHANNEL)

#define ADC_CLOCK_DIV   96.f
#define SAMPLES         200
#define ADC_ADJUST(x)   ((x) * 3.3f / (1 << 12u) - 1.65f)
#define ADC_MAX         3.3f
#define ADC_STEP        (3.3f / 5.f)

void microphone_init(void);
void sample_mic(void);
float mic_power(void);
uint8_t get_intensity(float v);

#ifdef __cplusplus
}
#endif

#endif // MICROPHONE_DMA_H
