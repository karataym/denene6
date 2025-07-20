#include "erain.h"
#include <string.h>

// ADC handle'ınızın global veya uygun bir yerden erişilebilir olduğundan emin olun
extern ADC_HandleTypeDef hadc1; // Örneğin ADC1 kullanıyorsanız

/**
  * @brief  ADC kanalına bağlı donanımın varlığını kontrol eder.
  * @param  channel ADC kanalı numarası (örneğin ADC_CHANNEL_1 için)
  * @param  threshold Donanımın bağlı olmadığını varsaymak için eşik değeri (0-4095 arası)
  * @param  high_if_unconnected Donanım bağlı değilken HIGH ise true, LOW ise false.
  * @retval true Donanım bağlı
  * @retval false Donanım bağlı değil
  * GPIO_InitStruct.Pin = GPIO_PIN_0;
GPIO_InitStruct.Mode = GPIO_MODE_INPUT; // Giriş olarak ayarla
GPIO_InitStruct.Pull = GPIO_PULLUP;     // Dahili pull-up direncini etkinleştir
GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  */
#define NUM_SAMPLES 20 // Ortalama alınacak okuma sayısı

static uint16_t CalculateAverage(uint32_t* buffer, uint8_t count) {
    if (count == 0) return 0;
    uint32_t sum = 0;
    for (uint8_t i = 0; i < count; i++) sum += buffer[i];
    return (uint16_t)(sum / count);
}

static uint8_t CalculatePercent(uint16_t value) {
    return (uint8_t)((value * 100UL) / ERAIN_MAX_ADC);
}

static ERAIN_Level_t DetermineLevel(ERAIN_Handle_t* handle, uint16_t value) {
    if (handle->config.adc_value_decreases_with_rain) {
        if (value <= handle->config.heavy_threshold) return ERAIN_LEVEL_HEAVY;
        if (value <= handle->config.moderate_threshold) return ERAIN_LEVEL_MODERATE;
        if (value <= handle->config.light_threshold) return ERAIN_LEVEL_LIGHT;
        return ERAIN_LEVEL_NONE;
    } else {
        if (value >= handle->config.heavy_threshold) return ERAIN_LEVEL_HEAVY;
        if (value >= handle->config.moderate_threshold) return ERAIN_LEVEL_MODERATE;
        if (value >= handle->config.light_threshold) return ERAIN_LEVEL_LIGHT;
        return ERAIN_LEVEL_NONE;
    }
}

HAL_StatusTypeDef ERAIN_Init(ERAIN_Handle_t* handle) {
    if (!handle || !handle->config.hadc) return HAL_ERROR;

    bool ok = handle->config.adc_value_decreases_with_rain
        ? (handle->config.heavy_threshold < handle->config.moderate_threshold &&
           handle->config.moderate_threshold < handle->config.light_threshold)
        : (handle->config.light_threshold < handle->config.moderate_threshold &&
           handle->config.moderate_threshold < handle->config.heavy_threshold);

    if (!ok) return HAL_ERROR;

    memset(handle->filter_buffer, 0, sizeof(handle->filter_buffer));
    handle->filter_index = 0;
    handle->sample_count = 0;

    handle->status = (ERAIN_Status_t){
        .initialized = true,
        .level = ERAIN_LEVEL_NONE,
        .role_active = false
    };

    return HAL_OK;
}

HAL_StatusTypeDef ERAIN_DeInit(ERAIN_Handle_t* handle) {
    if (!handle || !handle->status.initialized) return HAL_ERROR;
    handle->status.initialized = false;
    return HAL_OK;
}

HAL_StatusTypeDef ERAIN_Read(ERAIN_Handle_t* handle) {
    if (!handle || !handle->status.initialized) return HAL_ERROR;

    if (HAL_ADC_Start(handle->config.hadc) != HAL_OK) return HAL_ERROR;
    if (HAL_ADC_PollForConversion(handle->config.hadc, 100) != HAL_OK) return HAL_TIMEOUT;

    uint16_t adc_val = HAL_ADC_GetValue(handle->config.hadc);
    HAL_ADC_Stop(handle->config.hadc);

    handle->status.raw_adc_value = adc_val;
    handle->filter_buffer[handle->filter_index] = adc_val;
    handle->filter_index = (handle->filter_index + 1) % ERAIN_FILTER_SIZE;
    if (handle->sample_count < ERAIN_FILTER_SIZE) handle->sample_count++;

    uint16_t filtered = handle->sample_count >= ERAIN_FILTER_SIZE
        ? CalculateAverage(handle->filter_buffer, handle->sample_count)
        : adc_val;

    handle->status.filtered_adc_value = filtered;
    handle->status.percent = CalculatePercent(filtered);

    ERAIN_Level_t new_level = DetermineLevel(handle, filtered);
    if (new_level != handle->status.level) {
        handle->status.level = new_level;
        if (handle->config.rain_level_change_callback)
            handle->config.rain_level_change_callback(handle, new_level);
    }

    handle->status.last_update_tick = HAL_GetTick();
	if (ERAIN_IsRoleActive(handle)) {
		if (handle->config.relay_gpio != NULL) {
			HAL_GPIO_WritePin(handle->config.relay_gpio, handle->config.relay_pin, GPIO_PIN_SET);
		}
		if (handle->config.led_port != NULL) {
			HAL_GPIO_WritePin(handle->config.led_port, handle->config.led_pin, GPIO_PIN_SET);
		}
		if (handle->config.buzzer_port != NULL) {
			HAL_GPIO_TogglePin(handle->config.buzzer_port, handle->config.buzzer_pin);
		}
	} else {
		if (handle->config.relay_gpio != NULL) {
			HAL_GPIO_WritePin(handle->config.relay_gpio, handle->config.relay_pin, GPIO_PIN_RESET);
		}
		if (handle->config.led_port != NULL) {
			HAL_GPIO_WritePin(handle->config.led_port, handle->config.led_pin, GPIO_PIN_RESET);
		}
		if (handle->config.buzzer_port != NULL) {
			HAL_GPIO_WritePin(handle->config.buzzer_port, handle->config.buzzer_pin, GPIO_PIN_RESET);
		}
	}
    return HAL_OK;
}

HAL_StatusTypeDef ERAIN_SetThresholds(ERAIN_Handle_t* handle, uint16_t light, uint16_t moderate, uint16_t heavy) {
    if (!handle || !handle->status.initialized) return HAL_ERROR;

    bool ok = handle->config.adc_value_decreases_with_rain
        ? (heavy < moderate && moderate < light)
        : (light < moderate && moderate < heavy);

    if (!ok) return HAL_ERROR;

    handle->config.light_threshold = light;
    handle->config.moderate_threshold = moderate;
    handle->config.heavy_threshold = heavy;
    return HAL_OK;
}

// Getter Fonksiyonları

ERAIN_Level_t ERAIN_GetLevel(ERAIN_Handle_t* handle) {
    return (handle && handle->status.initialized) ? handle->status.level : ERAIN_LEVEL_NONE;
}

uint16_t ERAIN_GetRawValue(ERAIN_Handle_t* handle) {
    return (handle && handle->status.initialized) ? handle->status.raw_adc_value : 0;
}

uint16_t ERAIN_GetFilteredValue(ERAIN_Handle_t* handle) {
    return (handle && handle->status.initialized) ? handle->status.filtered_adc_value : 0;
}

uint8_t ERAIN_GetPercent(ERAIN_Handle_t* handle) {
    return (handle && handle->status.initialized) ? handle->status.percent : 0;
}

bool ERAIN_IsRoleActive(ERAIN_Handle_t* handle) {
    return (handle && handle->status.initialized) ? handle->status.role_active : false;
}

bool ERAIN_IsRaining(ERAIN_Handle_t* handle) {
    return (handle && handle->status.initialized) ? (handle->status.level != ERAIN_LEVEL_NONE) : false;
}
