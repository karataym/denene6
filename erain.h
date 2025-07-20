#ifndef INC_ERAIN_H_
#define INC_ERAIN_H_

#include "stm32f3xx_hal.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef HAL_ADC_MODULE_ENABLED

#define ERAIN_MAX_ADC                   4095U
#define ERAIN_FILTER_SIZE              8U
#define ERAIN_HEATER_TIMEOUT_MS   300000UL

typedef enum {
    ERAIN_LEVEL_NONE = 0,
    ERAIN_LEVEL_LIGHT,
    ERAIN_LEVEL_MODERATE,
    ERAIN_LEVEL_HEAVY
} ERAIN_Level_t;

struct ERAIN_Handle;

typedef void (*ERAIN_Callback_t)(struct ERAIN_Handle* handle, ERAIN_Level_t new_level);

typedef struct {
    ADC_HandleTypeDef* hadc;
    uint32_t channel;
    uint32_t Rank;
    uint32_t SingleDiff;
    uint32_t SamplingTime;
    uint32_t OffsetNumber;
    uint32_t Offset;
    uint16_t light_threshold;
    uint16_t moderate_threshold;
    uint16_t heavy_threshold;
    int16_t config_value[4];
    GPIO_TypeDef* relay_gpio;     // Role GPIO portu
	uint16_t relay_pin;           // Role pin numarası
	GPIO_TypeDef* led_port;       // LED GPIO portu (opsiyonel)
	uint16_t led_pin;             // LED pin numarası (opsiyonel)
	GPIO_TypeDef* buzzer_port;    // BUZZER GPIO portu (opsiyonel)
	uint16_t buzzer_pin;          // BUZZER pin numarası (opsiyonel)
    bool     adc_value_decreases_with_rain;
    ERAIN_Callback_t rain_level_change_callback;
} ERAIN_Config_t;

typedef struct {
    bool          initialized;
    ERAIN_Level_t level;
    uint16_t      raw_adc_value;
    uint16_t      filtered_adc_value;
    uint8_t       percent;
    bool          role_active;
    uint32_t      last_update_tick;
    uint32_t      last_heater_on_tick;
} ERAIN_Status_t;

typedef struct ERAIN_Handle {
    ERAIN_Config_t config;
    ERAIN_Status_t status;
    uint32_t       filter_buffer[ERAIN_FILTER_SIZE];
    uint8_t        filter_index;
    uint8_t        sample_count;
} ERAIN_Handle_t;

// API
HAL_StatusTypeDef ERAIN_Init(ERAIN_Handle_t* handle);
HAL_StatusTypeDef ERAIN_DeInit(ERAIN_Handle_t* handle);
HAL_StatusTypeDef ERAIN_Read(ERAIN_Handle_t* handle);
HAL_StatusTypeDef ERAIN_SetThresholds(ERAIN_Handle_t* handle, uint16_t light, uint16_t moderate, uint16_t heavy);

ERAIN_Level_t ERAIN_GetLevel(ERAIN_Handle_t* handle);
uint16_t ERAIN_GetRawValue(ERAIN_Handle_t* handle);
uint16_t ERAIN_GetFilteredValue(ERAIN_Handle_t* handle);
uint8_t  ERAIN_GetPercent(ERAIN_Handle_t* handle);
bool     ERAIN_IsRoleActive(ERAIN_Handle_t* handle);
bool     ERAIN_IsRaining(ERAIN_Handle_t* handle);


#endif // HAL_ADC_MODULE_ENABLED
#endif /* INC_ERAIN_H_ */
