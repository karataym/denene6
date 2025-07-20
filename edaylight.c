/*
 * edaylight.c
 *
 *  Created on: Nov 3, 2024
 *      Author: mehme
*/

#include "edaylight.h"



Lighting_pozition light_pozition = 1;

//void default_dayligth_params(void) {
//	DL_config[0] = 400;
//	DL_config[1] = 1000;
//	DL_config[2] = 2000;
//	DL_config[3] = 3000;
//}
EDAYLIGHT_Handle_t daylight;
//EDAYLIGHT_Config_t daylight_config;

//Daylight_info daylight_bilgi;
// Sabitler
#define EDAYLIGHT_MAX_ADC      4095  // 12-bit ADC için maksimum değer
#define EDAYLIGHT_FILTER_SIZE  8     // Hareketli ortalama filtre boyutu
#define EDAYLIGHT_MIN_SAMPLES  4     // Minimum örnek sayısı

//#ifdef HAL_ADC_MODULE_ENABLED
bool EDAYLIGHT_IsSensorDisconnected(EDAYLIGHT_Handle_t* handle) {
    if (!handle || !handle->status.initialized) return true;
//    uint16_t val = handle->status.raw_adc_value;
//    // Tipik aralık dışı değerler (ayarlanabilir)
//    if (val < 100 || val > 4000) {
//        return true; // Modül yok gibi
//    }
//    // Hızlı sapmalar tespit edilebilir
//    if (handle->sample_count >= Daylight_FILTER_SIZE) {
//        uint16_t filtered = handle->status.filtered_adc_value;
//        if (abs((int)filtered - (int)val) > 800) {
//            return true;
//        }
//    }
    return false;
}
// Yardımcı fonksiyonlar
static uint16_t CalculateAverage(uint32_t* buffer, uint8_t size) {
    uint32_t sum = 0;
    for (uint8_t i = 0; i < size; i++) {
        sum += buffer[i];
    }
    return (uint16_t)(sum / size);
}

static uint8_t CalculatePercent(uint16_t value) {
    return (uint8_t)((value * 100) / EDAYLIGHT_MAX_ADC);
}

static EDAYLIGHT_Level_t DetermineLevel(EDAYLIGHT_Handle_t* handle, uint16_t value) {
    //uint8_t percent = CalculatePercent(value);

    if (value <= handle->config.dark_threshold) {
        return EDAYLIGHT_LEVEL_DARK;
    } else if (value <= handle->config.dim_threshold) {
        return EDAYLIGHT_LEVEL_DIM;
    } else if (value <= handle->config.moderate_threshold) {
        return EDAYLIGHT_LEVEL_MODERATE;
    } else if (value <= handle->config.bright_threshold) {
        return EDAYLIGHT_LEVEL_BRIGHT;
    } else {
        return EDAYLIGHT_LEVEL_VERY_BRIGHT;
    }
}

uint16_t EDAYLIGHT_threshold_calc_percent(uint8_t in_value) {
	if (in_value > 100) {
		in_value = 100;
	}
	float value = in_value / 100;
	return (EDAYLIGHT_MAX_ADC * value);
}
// Ana fonksiyonlar , EDAYLIGHT_Config_t* config
HAL_StatusTypeDef EDAYLIGHT_Init(EDAYLIGHT_Handle_t* handle) {
    if (!handle->config.hadc) {
		return HAL_ERROR;
	}
    // Varsayılan eşikleri ayarla
    if (handle->config.dark_threshold == 0) {
        handle->config.dark_threshold = EDAYLIGHT_threshold_calc_percent(10);  // 10%
    }
    if (handle->config.dim_threshold == 0) {
        handle->config.dim_threshold = EDAYLIGHT_threshold_calc_percent(30);;   // 30%
    }
    if (handle->config.moderate_threshold == 0) {
        handle->config.moderate_threshold = EDAYLIGHT_threshold_calc_percent(60); // 60%
    }
    if (handle->config.bright_threshold == 0) {
        handle->config.bright_threshold = EDAYLIGHT_threshold_calc_percent(90);  // 90%
    }

    // Durum bilgilerini sıfırla
    handle->status.initialized = 1;
    handle->status.level = EDAYLIGHT_LEVEL_DARK;
    handle->status.percent = 0;
    handle->status.last_update = 0;
    handle->status.sample_count = 0;
    handle->filter_index = 0;
    //daylight_config.callback = LightLevelChanged;
    // Filtre buffer'ını sıfırla
    for (uint8_t i = 0; i < EDAYLIGHT_FILTER_SIZE; i++) {
        handle->filter_buffer[i] = 0;
    }
    return HAL_OK;
}

HAL_StatusTypeDef EDAYLIGHT_DeInit(EDAYLIGHT_Handle_t* handle) {
    if (!handle) {
        return HAL_ERROR;
    }

    handle->status.initialized = 0;
    return HAL_OK;
}

HAL_StatusTypeDef EDAYLIGHT_Read(EDAYLIGHT_Handle_t* handle) {
    if (!handle->status.initialized) {
        return HAL_ERROR;
    }

    HAL_StatusTypeDef status;
    uint32_t raw_value;

    // ADC dönüşümünü başlat
    status = HAL_ADC_Start(handle->config.hadc);
    if (status != HAL_OK) return status;

    // Dönüşümün bitmesini bekle
    status = HAL_ADC_PollForConversion(handle->config.hadc, 100);
    if (status != HAL_OK) return status;

    // ADC değerini oku
    raw_value = HAL_ADC_GetValue(handle->config.hadc);

    status = HAL_ADC_Stop(handle->config.hadc);
	if (status != HAL_OK) return status;

    // Filtre buffer'ına ekle
    handle->filter_buffer[handle->filter_index] = raw_value;
    handle->filter_index = (handle->filter_index + 1) % EDAYLIGHT_FILTER_SIZE;
    handle->status.sample_count++;

    // Yeterli örnek varsa değerleri güncelle
    if (handle->status.sample_count >= EDAYLIGHT_MIN_SAMPLES) {
        uint16_t filtered_value = CalculateAverage(handle->filter_buffer, EDAYLIGHT_FILTER_SIZE);
        EDAYLIGHT_Level_t new_level = DetermineLevel(handle, filtered_value);

        // Değerleri güncelle
        handle->daylight_info.daylight_analog_value = filtered_value;
        handle->status.percent = CalculatePercent(filtered_value);

        // Seviye değiştiyse callback'i çağır
        if (new_level != handle->status.level) {
            handle->status.level = new_level;
//            if (handle->config.daylight_level_change_callback) {
//                handle->config.daylight_level_change_callback(handle, new_level);
//            }
        }

        handle->status.last_update = HAL_GetTick();
    }
    handle->daylight_info.daylight_analog_value = EDAYLIGHT_GetRawValue(&daylight);// raw_value: Ham Okunan ADC değeri
    handle->daylight_info.daylight_status = EDAYLIGHT_GetLevel(&daylight);   // Gün ışığı durumu
    handle->daylight_info.daylight_percent = EDAYLIGHT_GetPercent(&daylight);	// : Işık seviyesi yüzdesi (%0-100).
    handle->daylight_info.daylight_level = EDAYLIGHT_GetLevel(&daylight); // //  Mevcut ışık seviyesi.
    handle->daylight_info.daylight_relay_status =  EDAYLIGHT_IsDark(&daylight);  // Röle durumu (0: Kapalı, 1: Açık)
    if (handle->daylight_info.daylight_relay_status) {
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

uint16_t EDAYLIGHT_GetRawValue(EDAYLIGHT_Handle_t* handle) {
    return handle->daylight_info.daylight_analog_value;
}

uint8_t EDAYLIGHT_GetPercent(EDAYLIGHT_Handle_t* handle) {
    return handle->status.percent;
}

EDAYLIGHT_Level_t EDAYLIGHT_GetLevel(EDAYLIGHT_Handle_t* handle) {
    return handle->status.level;
}

HAL_StatusTypeDef EDAYLIGHT_Calibrate(EDAYLIGHT_Handle_t* handle) {
	if (!handle->status.initialized) {
        return HAL_ERROR;
    }

    // Birkaç örnek al
    uint32_t sum = 0;
    uint8_t samples = 10;

    for (uint8_t i = 0; i < samples; i++) {
        HAL_StatusTypeDef status = EDAYLIGHT_Read(handle);
        if (status != HAL_OK) return status;
        sum += handle->daylight_info.daylight_analog_value;
        HAL_Delay(100);  // Örnekler arası bekle
    }

    // Ortalama değere göre eşikleri ayarla
    uint16_t avg = (uint16_t)(sum / samples);
    handle->config.dark_threshold = (uint16_t)(avg * 0.2f);
    handle->config.dim_threshold = (uint16_t)(avg * 0.4f);
    handle->config.moderate_threshold = (uint16_t)(avg * 0.6f);
    handle->config.bright_threshold = (uint16_t)(avg * 0.8f);

    return HAL_OK;
}

HAL_StatusTypeDef EDAYLIGHT_SetThresholds(EDAYLIGHT_Handle_t* handle, uint8_t dark, uint8_t dim, uint8_t moderate, uint8_t bright) {
    if (!handle->status.initialized) {
        return HAL_ERROR;
    }

    // Eşiklerin sıralı olduğunu kontrol et
    if (dark >= dim || dim >= moderate || moderate >= bright) {
        return HAL_ERROR;
    }

    handle->config.dark_threshold = EDAYLIGHT_threshold_calc_percent(dark);
    handle->config.dim_threshold = EDAYLIGHT_threshold_calc_percent(dim);
    handle->config.moderate_threshold = EDAYLIGHT_threshold_calc_percent(moderate);
    handle->config.bright_threshold = EDAYLIGHT_threshold_calc_percent(bright);

    return HAL_OK;
}

uint8_t EDAYLIGHT_IsDark(EDAYLIGHT_Handle_t* handle) {
	if (light_pozition == 1 ) {
		return (handle->status.level == EDAYLIGHT_LEVEL_DARK) || (handle->status.level == EDAYLIGHT_LEVEL_DIM);
	} else if (light_pozition == 2 ) {
		return (handle->status.level == EDAYLIGHT_LEVEL_DARK);
	}
	return (handle->status.level == EDAYLIGHT_LEVEL_DARK);
}

uint8_t EDAYLIGHT_IsBright(EDAYLIGHT_Handle_t* handle) {
	if (light_pozition == 1 ) {
		return (handle->status.level >= EDAYLIGHT_LEVEL_MODERATE);
	} else if (light_pozition == 2 ) {
		return (handle->status.level >= EDAYLIGHT_LEVEL_BRIGHT);
	}
    return (handle->status.level >= EDAYLIGHT_LEVEL_BRIGHT);
}

EDAYLIGHT_Status_t EDAYLIGHT_GetStatus(EDAYLIGHT_Handle_t* handle) {
    return handle->status;
}
//#endif
/*

static Daylight_status daylight_status = COK_KARANLIK;
static Lighting_pozition lighting_pozition = KARARIYOR;
Daylight_info daylight_bilgi;
static uint32_t daylight_onceki_adc_degeri = 0;
static uint32_t daylight_onceki_status_adc_degeri = 0;
static uint32_t daylight_histeri = 100;
int16_t DL_config[4]; //LC_1 = 500, LC_2 = 1500, LC_3 = 2500, LC_4 = 3500;

void default_dayligth_params(void) {
	DL_config[0] = 400;
	DL_config[1] = 1000;
	DL_config[2] = 2000;
	DL_config[3] = 3000;
}
Daylight_info read_daylight(uint32_t adc_degeri) {
	static Daylight_info current_Daylight_info = {TAM_AYDINLIK, KARARIYOR, 0};
	    if (adc_degeri > daylight_onceki_adc_degeri + daylight_histeri) {
	    	current_Daylight_info.dayligh_pozision = AYDINLANIYOR;
	    } else if (adc_degeri < daylight_onceki_adc_degeri - daylight_histeri) {
	    	current_Daylight_info.dayligh_pozision = KARARIYOR;
	    }
	    if (adc_degeri > daylight_onceki_status_adc_degeri + daylight_histeri || adc_degeri < daylight_onceki_adc_degeri - daylight_histeri) {
	        if (adc_degeri < DL_config[0]) {
	        	current_Daylight_info.dayligh_status = COK_KARANLIK;
	        	current_Daylight_info.dayligh_relay_status = 0;
	        } else if (adc_degeri < DL_config[1]) {
	        	current_Daylight_info.dayligh_status = KARANLIK;
	        	current_Daylight_info.dayligh_relay_status = 0;
	        } else if (adc_degeri < DL_config[2]) {
	        	current_Daylight_info.dayligh_status = GUN_AGIRISI;
	        	current_Daylight_info.dayligh_relay_status = 1;
	        } else if (adc_degeri < DL_config[3]) {
	        	current_Daylight_info.dayligh_status = AYDINLIK;
	        	current_Daylight_info.dayligh_relay_status = 1;
	        } else {
	        	current_Daylight_info.dayligh_status = TAM_AYDINLIK;
	        	current_Daylight_info.dayligh_relay_status = 1;
	        }
	    }
	daylight_onceki_adc_degeri = adc_degeri;
	daylight_onceki_status_adc_degeri = current_Daylight_info.dayligh_status;
	return current_Daylight_info;
}
*/
