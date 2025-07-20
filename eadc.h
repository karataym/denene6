/*
 * eadc.h
 *
 *  Created on: Jun 22, 2025
 *      Author: mehme
 */

#ifndef ELIB_EADC_H_
#define ELIB_EADC_H_

#include "stm32f3xx_hal.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef HAL_ADC_MODULE_ENABLED

#define NUM_SAMPLES 20 // Ortalama alınacak okuma sayısıL
extern ADC_ChannelConfTypeDef eADCConfig;

bool eADC_HardwareConnected(ADC_HandleTypeDef* ehadc, uint16_t threshold, bool high_if_unconnected);

#endif /* HAL_ADC_MODULE_ENABLED */

#endif /* ELIB_EADC_H_ */
