/*
 * eadc.c
 *
 *  Created on: Jun 22, 2025
 *      Author: mehme
 */

#include "eadc.h"

ADC_ChannelConfTypeDef eADCConfig = {0};
/*
 * ADC_HandleTypeDef* hadc;
    uint32_t channel;
    uint32_t Rank;
    uint32_t SingleDiff;
    uint32_t SamplingTime;
    uint32_t OffsetNumber;
    uint32_t Offset;
 */

bool eADC_HardwareConnected(ADC_HandleTypeDef* ehadc, uint16_t threshold, bool high_if_unconnected) {
    uint32_t adc_raw_sum = 0;
    uint16_t adc_raw_average = 0;
    if (HAL_ADC_ConfigChannel(ehadc, &eADCConfig) != HAL_OK) { // Hata yönetimi
        return false; // Konfigürasyon hatası, bağlı değil varsayılabilir
    }
    for (int i = 0; i < NUM_SAMPLES; i++) {
        HAL_ADC_Start(ehadc);
        if (HAL_ADC_PollForConversion(ehadc, 100) == HAL_OK) { // 100ms timeout
            adc_raw_sum += HAL_ADC_GetValue(ehadc);
        } else {
            HAL_ADC_Stop(ehadc);
            return false;
        }
        HAL_ADC_Stop(ehadc); // Her okumadan sonra ADC'yi durdurmak genellikle iyi bir uygulamadır.
    }
    adc_raw_average = adc_raw_sum / NUM_SAMPLES;
    if (high_if_unconnected) { // Donanım bağlı değilken yüksek okuma bekleniyorsa
        return (adc_raw_average < threshold); // Ortalama okuma eşikten düşükse bağlı demektir
    } else { // Donanım bağlı değilken düşük okuma bekleniyorsa
        return (adc_raw_average > threshold); // Ortalama okuma eşikten yüksekse bağlı demektir
    }
}
