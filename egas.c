/*
 *  egas.c
 *
 *  Created on: Jun 15, 2025
 *      Author: mehmet ve süper chatCPT USTASI (KISACA HOCAM)
 */

#include "egas.h"

HAL_StatusTypeDef EGAS_Init(EGAS_Handle_t* handle, EGAS_Config_t* config) {
    if (!handle || !config) return HAL_ERROR;
    // Konfigürasyon ayarlarını sürücüye aktar
    handle->config = *config;
    handle->status.initialized = true;
    handle->status.level = EGAS_LEVEL_SAFE;
    handle->filter_index = 0;
    handle->sample_count = 0;
    // ADC yapılandırması (runtime ayarı yapılabilir)
    ADC_ChannelConfTypeDef sConfig = {0};
    sConfig.Channel = config->channel;
    sConfig.Rank = config->Rank;
    sConfig.SingleDiff = config->SingleDiff;
    sConfig.SamplingTime = config->SamplingTime;
    sConfig.OffsetNumber = config->OffsetNumber;
    sConfig.Offset = config->Offset;
    handle->status.initialized = true;
    handle->status.level = EGAS_LEVEL_SAFE;
    handle->filter_index = 0;
    handle->sample_count = 0;
    if (HAL_ADC_ConfigChannel(config->hadc, &sConfig) != HAL_OK) {
        return HAL_ERROR;
    }
    return HAL_OK;
}

HAL_StatusTypeDef EGAS_Read(EGAS_Handle_t* handle) {
    if (!handle || !handle->config.hadc) return HAL_ERROR;
    
    // ADC değeri okuma
    HAL_ADC_Start(handle->config.hadc);
    HAL_ADC_PollForConversion(handle->config.hadc, 10);
    uint16_t new_value = HAL_ADC_GetValue(handle->config.hadc);
    HAL_ADC_Stop(handle->config.hadc);
    // Filtreleme (ortalama alma)
    handle->filter_buffer[handle->filter_index++] = new_value;
    if (handle->filter_index >= EGAS_FILTER_SIZE) handle->filter_index = 0;
    
    uint32_t sum = 0;
    for (uint8_t i = 0; i < EGAS_FILTER_SIZE; i++) {
        sum += handle->filter_buffer[i];
    }
    handle->status.filtered_adc_value = (uint16_t)(sum / EGAS_FILTER_SIZE);

    // Eşik değerlerine göre seviyeyi belirle
    if (handle->status.filtered_adc_value >= handle->config.critical_threshold) {
        handle->status.level = EGAS_LEVEL_CRITICAL;
        handle->status.role_active = true;
    } else if (handle->status.filtered_adc_value >= handle->config.alert_threshold) {
        handle->status.level = EGAS_LEVEL_ALERT;
    } else if (handle->status.filtered_adc_value >= handle->config.warning_threshold) {
        handle->status.level = EGAS_LEVEL_WARNING;
    } else {
        handle->status.level = EGAS_LEVEL_SAFE;
        handle->status.role_active = false;
    }
	if (handle->status.role_active) {
		if (handle->config.relay_gpio != NULL) {
			HAL_GPIO_WritePin(handle->config.relay_gpio, handle->config.relay_pin, GPIO_PIN_SET);
		}
		if (handle->config.led_port != NULL) {
			HAL_GPIO_WritePin(handle->config.led_port, handle->config.led_pin, GPIO_PIN_SET);
		}
		if (handle->config.buzzer_port != NULL) {
			if (handle->status.level == EGAS_LEVEL_CRITICAL) {
				HAL_GPIO_WritePin(handle->config.buzzer_port, handle->config.buzzer_pin, GPIO_PIN_SET);
			} else {
				HAL_GPIO_TogglePin(handle->config.buzzer_port, handle->config.buzzer_pin);
			}
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

HAL_StatusTypeDef EGAS_SetThresholds(EGAS_Handle_t* handle, uint16_t warning, uint16_t alert, uint16_t critical) {
    if (!handle) return HAL_ERROR;
    
    handle->config.warning_threshold = warning;
    handle->config.alert_threshold = alert;
    handle->config.critical_threshold = critical;
    
    return HAL_OK;
}
bool EGAZ_IsRoleActive(EGAS_Handle_t* handle) {
    return handle->status.role_active == 1;
}

EGAS_Level_t EGAS_GetLevel(EGAS_Handle_t* handle) {
    return handle ? handle->status.level : EGAS_LEVEL_SAFE;
}

uint16_t EGAS_GetRawValue(EGAS_Handle_t* handle) {
    return handle ? handle->status.raw_adc_value : 0;
}

uint16_t EGAS_GetFilteredValue(EGAS_Handle_t* handle) {
    return handle ? handle->status.filtered_adc_value : 0;
}

bool EGAS_IsCritical(EGAS_Handle_t* handle) {
    return handle ? (handle->status.level == EGAS_LEVEL_CRITICAL) : false;
}

/*
EGAS_Handle_t egas_handle;  // Gaz Sensörü Sürücüsü
EGAS_Config_t egas_config;  // Konfigürasyon Parametreleri



Örnekleme Süresi	Avantaj	Dezavantaj
19 Cycles	        Hızlı dönüş sağlar	Gürültüye daha açık
61 Cycles	        Dengeli, çoğu uygulama için iyi	Orta hızda örnekleme
181 Cycles	        Daha güvenilir sonuç, düşük gürültü	Daha uzun ADC dönüş süresi
601 Cycles	        En temiz okuma, hassas ölçüm	En yavaş dönüş süresi


void EGAS_Config_Init() {
    egas_config.hadc = &hadc1;
    egas_config.channel = ADC_CHANNEL_2; // PC2 için
    egas_config.rank = ADC_REGULAR_RANK_1;
    egas_config.single_diff = ADC_SINGLE_ENDED;
    egas_config.sampling_time = ADC_SAMPLETIME_181CYCLES;
    egas_config.offset_number = ADC_OFFSET_NONE;
    egas_config.offset = 0;
    
    // Varsayılan eşik değerleri (Optimizasyon gerekirse değiştirilebilir)
    egas_config.warning_threshold = 500;
    egas_config.alert_threshold = 1500;
    egas_config.critical_threshold = 2500;
    egas_config.adc_value_increases_with_gas = true;

    if (EGAS_Init(&egas_handle, &egas_config) != HAL_OK) {
        printf("EGAS Başlatma Hatası!\n");
    }
}


if (EGAS_Read(&egas_handle) == HAL_OK) {
            EGAS_Level_t level = EGAS_GetLevel(&egas_handle);
            
            if (level == EGAS_LEVEL_CRITICAL) {
                printf("KRİTİK! Gaz seviyesi çok yüksek!\n");
                // Röle ve buzzer tetikle
            } else if (level == EGAS_LEVEL_ALERT) {
                printf("İkaz! Gaz seviyesi yükseliyor!\n");
            } else if (level == EGAS_LEVEL_WARNING) {
                printf("Uyarı! Düşük seviyede gaz algılandı.\n");
            } else {
                printf("Güvenli: Normal gaz seviyesi\n");
            }
        }
        HAL_Delay(500); // 500ms bekleme
    }
    */
