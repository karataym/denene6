/*
 *  egas.h
 *
 *  Created on: Jun 15, 2025
 *      Author: mehmet ve süper chatCPT USTASI (KISACA HOCAM)
 */

#ifndef INC_EGAS_H_
#define INC_EGAS_H_

#include "stm32f3xx_hal.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef HAL_ADC_MODULE_ENABLED

#define EGAS_MAX_ADC              4095U
#define EGAS_FILTER_SIZE          10U

typedef enum {
    EGAS_LEVEL_SAFE = 0,
    EGAS_LEVEL_WARNING,
    EGAS_LEVEL_ALERT,
    EGAS_LEVEL_CRITICAL
} EGAS_Level_t;

struct EGAS_Handle;
typedef void (*EGAS_Callback_t)(struct EGAS_Handle* handle, EGAS_Level_t new_level);

typedef struct {
    ADC_HandleTypeDef* hadc;
    uint32_t channel;
    uint32_t Rank;
    uint32_t SingleDiff;
    uint32_t SamplingTime;
    uint32_t OffsetNumber;
    uint32_t Offset;
    uint16_t warning_threshold;
    uint16_t alert_threshold;
    uint16_t critical_threshold;
    GPIO_TypeDef* relay_gpio;     // Role GPIO portu
	uint16_t relay_pin;           // Role pin numarası
	GPIO_TypeDef* led_port;       // LED GPIO portu (opsiyonel)
	uint16_t led_pin;             // LED pin numarası (opsiyonel)
	GPIO_TypeDef* buzzer_port;    // BUZZER GPIO portu (opsiyonel)
	uint16_t buzzer_pin;          // BUZZER pin numarası (opsiyonel)
	int16_t config_value[4];
	bool     adc_value_increases_with_gas;
    EGAS_Callback_t gaz_level_change_callback;
} EGAS_Config_t;

typedef struct {
    bool          initialized;
    EGAS_Level_t  level;
    uint16_t      raw_adc_value;
    uint16_t      filtered_adc_value;
    uint8_t       percent;
    bool          role_active;
} EGAS_Status_t;

typedef struct EGAS_Handle {
    EGAS_Config_t config;
    EGAS_Status_t status;
    uint16_t      filter_buffer[EGAS_FILTER_SIZE];
    uint8_t       filter_index;
    uint8_t       sample_count;
} EGAS_Handle_t;
 
// API
HAL_StatusTypeDef EGAS_Init(EGAS_Handle_t* handle, EGAS_Config_t* config);
HAL_StatusTypeDef EGAS_Read(EGAS_Handle_t* handle);
HAL_StatusTypeDef EGAS_SetThresholds(EGAS_Handle_t* handle, uint16_t warning, uint16_t alert, uint16_t critical);
EGAS_Level_t      EGAS_GetLevel(EGAS_Handle_t* handle);
bool EGAZ_IsRoleActive(EGAS_Handle_t* handle);
uint16_t          EGAS_GetRawValue(EGAS_Handle_t* handle);
uint16_t          EGAS_GetFilteredValue(EGAS_Handle_t* handle);
bool              EGAS_IsCritical(EGAS_Handle_t* handle);

#endif // HAL_ADC_MODULE_ENABLED
#endif /* INC_EGAS_H_ */
