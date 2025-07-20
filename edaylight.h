/*
 * daylight.h
 *
 *  Created on: Nov 3, 2024
 *      Author: mehme
 */

#ifndef ELIB_EDAYLIGHT_H_
#define ELIB_EDAYLIGHT_H_

#define DL_Modbus_config_0 6
#define DL_Modbus_config_1 7
#define DL_Modbus_config_2 8
#define DL_Modbus_config_3 9
#define DL_Modbus_analogs_adc_values 10
#define DL_Modbus_analogs_dayligh_status 11
#define DL_Modbus_analogs_relay_status 12


#include "stm32f3xx.h"
//#include "eprogs.h"
#include <stdint.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>


// Gün ışığı seviye tanımları
typedef enum {
    EDAYLIGHT_LEVEL_DARK = 0,     // Karanlık (<10%)
    EDAYLIGHT_LEVEL_DIM,          // Loş (10-30%)
    EDAYLIGHT_LEVEL_MODERATE,     // Orta (30-60%)
    EDAYLIGHT_LEVEL_BRIGHT,       // Parlak (60-90%)
    EDAYLIGHT_LEVEL_VERY_BRIGHT   // Çok parlak (>90%)
} EDAYLIGHT_Level_t;

typedef struct {
	EDAYLIGHT_Level_t daylight_status;   // Gün ışığı durumu
	uint8_t daylight_percent; 			 // : Işık seviyesi yüzdesi (%0-100).
    uint8_t daylight_relay_status;       // Röle durumu (0: Kapalı, 1: Açık)
    uint16_t daylight_analog_value;      // raw_value: Ham ADC değeri.  Okunan ADC değeri
    uint8_t daylight_level;				 //  Mevcut ışık seviyesi.
    uint16_t config_value[4];
} Daylight_info;

typedef enum {
    LIGHTING_POSITION_UNKNOWN = 0,  // Bilinmiyor
    LIGHTING_POSITION_INDOOR,       // İç mekan
    LIGHTING_POSITION_OUTDOOR       // Dış mekan
} Lighting_pozition;
#define Daylight_MAX_ADC                   4095U
#define Daylight_FILTER_SIZE              8U
#define Daylight_HEATER_TIMEOUT_MS   300000UL
extern Lighting_pozition light_pozition;
extern Daylight_info daylight_bilgi;

struct EDAYLIGHT_Handle;
typedef void (*EDAYLIGHT_Callback_t)(struct EDAYLIGHT_Handle* handle, EDAYLIGHT_Level_t new_level);
// Yapılandırma yapısı
typedef struct {
	#ifdef HAL_ADC_MODULE_ENABLED
    ADC_HandleTypeDef* hadc;      // ADC handle
	#endif
    uint32_t channel;             // ADC kanalı
    uint32_t Rank;
    uint32_t SingleDiff; // = ADC_SINGLE_ENDED;
    uint32_t SamplingTime; // = ADC_SAMPLETIME_61CYCLES_5; // ADC_SAMPLETIME_19CYCLES_5; // Örnekleme süresi
    uint32_t OffsetNumber; // = ADC_OFFSET_NONE;
    uint32_t Offset;
    uint16_t dark_threshold;      // Karanlık eşiği
    uint16_t dim_threshold;       // Loş eşiği
    uint16_t moderate_threshold;  // Orta eşiği
    uint16_t bright_threshold;    // Parlak eşiği
    GPIO_TypeDef* relay_gpio;     // Role GPIO portu
	uint16_t relay_pin;           // Role pin numarası
	GPIO_TypeDef* led_port;       // LED GPIO portu (opsiyonel)
	uint16_t led_pin;             // LED pin numarası (opsiyonel)
	GPIO_TypeDef* buzzer_port;    // BUZZER GPIO portu (opsiyonel)
	uint16_t buzzer_pin;          // BUZZER pin numarası (opsiyonel)
	int16_t config_value[4];
	EDAYLIGHT_Callback_t daylight_level_change_callback; // void (*callback)(EDAYLIGHT_Level_t); // Seviye değişim callback'i
} EDAYLIGHT_Config_t;

//extern EDAYLIGHT_Config_t daylight_config;

// Durum yapısı
typedef struct {
	uint8_t initialized;             // Başlatma durumu
    EDAYLIGHT_Level_t level;      // Mevcut seviye
    uint8_t percent;             // Yüzde olarak değer (0-100)
    uint32_t last_update;        // Son güncelleme zamanı
    uint32_t sample_count;       // Örnek sayısı
} EDAYLIGHT_Status_t;

// Handle yapısı
typedef struct EDAYLIGHT_Handle {
    EDAYLIGHT_Config_t config;    // Yapılandırma
    EDAYLIGHT_Status_t status;    // Durum
    Daylight_info daylight_info;
    uint32_t filter_buffer[8];    // Filtre buffer'ı
    uint8_t filter_index;         // Filtre indeksi
} EDAYLIGHT_Handle_t;

//#ifdef HAL_ADC_MODULE_ENABLED
// Fonksiyon prototipleri , EDAYLIGHT_Config_t* config
HAL_StatusTypeDef EDAYLIGHT_Init(EDAYLIGHT_Handle_t* handle);
HAL_StatusTypeDef EDAYLIGHT_DeInit(EDAYLIGHT_Handle_t* handle);

// Temel fonksiyonlar
HAL_StatusTypeDef EDAYLIGHT_Read(EDAYLIGHT_Handle_t* handle);

// Veri alma fonksiyonları
uint16_t EDAYLIGHT_GetRawValue(EDAYLIGHT_Handle_t* handle);
uint8_t EDAYLIGHT_GetPercent(EDAYLIGHT_Handle_t* handle);
EDAYLIGHT_Level_t EDAYLIGHT_GetLevel(EDAYLIGHT_Handle_t* handle);

// Kalibrasyon fonksiyonları
HAL_StatusTypeDef EDAYLIGHT_Calibrate(EDAYLIGHT_Handle_t* handle);
HAL_StatusTypeDef EDAYLIGHT_SetThresholds(EDAYLIGHT_Handle_t* handle, uint8_t dark, uint8_t dim, uint8_t moderate, uint8_t bright);

// Durum fonksiyonları
uint8_t EDAYLIGHT_IsDark(EDAYLIGHT_Handle_t* handle);
uint8_t EDAYLIGHT_IsBright(EDAYLIGHT_Handle_t* handle);
EDAYLIGHT_Status_t EDAYLIGHT_GetStatus(EDAYLIGHT_Handle_t* handle);

uint16_t EDAYLIGHT_threshold_calc_percent(uint8_t in_value);
bool EDAYLIGHT_IsSensorDisconnected(EDAYLIGHT_Handle_t* handle);
//#endif

/*
* @brief Gün Işığı Sensörü Driver'ı
*
* Bu driver, ortam ışık seviyesini ölçmek için kullanılır.
* Özellikler:
* - 5 farklı ışık seviyesi (karanlık, loş, orta, parlak, çok parlak)
* - ADC ile analog okuma
* - Hareketli ortalama filtresi (8 örnek)
* - Otomatik kalibrasyon
* - Ayarlanabilir eşikler
* - Seviye değişim callback'i
*
* @note Kullanmadan önce ADC'nin konfigüre edilmiş olması gerekir
*
* @example Örnek Kullanım
* @code
* // Callback fonksiyonu
* void LightLevelChanged(EDAYLIGHT_Level_t new_level) {
*     switch(new_level) {
*         case EDAYLIGHT_LEVEL_DARK:
*             // Işıkları aç
*             break;
*         case EDAYLIGHT_LEVEL_BRIGHT:
*             // Işıkları kapat
*             break;
*     }
* }
*
* // Handle ve config yapılarını oluştur
* EDAYLIGHT_Handle_t daylight;
* EDAYLIGHT_Config_t config = {
*     .hadc = &hadc1,
*     .channel = ADC_CHANNEL_1,
*     .sample_time = ADC_SAMPLETIME_7CYCLES_5,
*     .callback = LightLevelChanged
* };
*
* // Driver'ı başlat
* EDAYLIGHT_Init(&daylight, &config);
*
* // Opsiyonel: Kalibrasyon yap
* EDAYLIGHT_Calibrate(&daylight);
*
* // Ana döngüde
* while (1) {
*     // Sensörü güncelle
*     EDAYLIGHT_Update(&daylight);
*
*     // Mevcut değerleri oku
*     uint16_t raw = EDAYLIGHT_GetRawValue(&daylight);
*     uint8_t percent = EDAYLIGHT_GetPercent(&daylight);
*     EDAYLIGHT_Level_t level = EDAYLIGHT_GetLevel(&daylight);
*
*     // Durum kontrolü
*     if (EDAYLIGHT_IsDark(&daylight)) {
*         // Karanlık modu işlemleri
*     }
*
*     HAL_Delay(100);
* }
* @endcode
*/




/*
typedef enum {  // Daylight durumları
    COK_KARANLIK,
    KARANLIK,
    GUN_AGIRISI,
    AYDINLIK,
    TAM_AYDINLIK
} Daylight_status;

#define DL_Modbus_config_0 6
#define DL_Modbus_config_1 7
#define DL_Modbus_config_2 8
#define DL_Modbus_config_3 9
#define DL_Modbus_analogs_adc_values 10
#define DL_Modbus_analogs_dayligh_status 11
#define DL_Modbus_analogs_relay_status 12

typedef enum {  // Aydınlanma veya kararma durumları
    KARARIYOR,
    AYDINLANIYOR
} Lighting_pozition;
typedef struct { // Yapı tanımı
    Daylight_status dayligh_status;
    Lighting_pozition dayligh_pozision;
    uint8_t dayligh_relay_status;
} Daylight_info;

// Işık durumu enum'u
typedef enum {
    DAYLIGHT_STATUS_UNKNOWN = 0,  // Bilinmiyor
    DAYLIGHT_STATUS_NORMAL,       // Normal çalışma
    DAYLIGHT_STATUS_LOW,          // Düşük ışık
    DAYLIGHT_STATUS_HIGH          // Yüksek ışık
} Daylight_status;

// Aydınlatma konumu enum'u
typedef enum {
    LIGHTING_POSITION_UNKNOWN = 0,  // Bilinmiyor
    LIGHTING_POSITION_INDOOR,       // İç mekan
    LIGHTING_POSITION_OUTDOOR       // Dış mekan
} Lighting_pozition;

// Gün ışığı bilgileri yapısı


// Global Daylight_info örneği
Daylight_info daylight_info;





extern int16_t DL_config[4];
extern Daylight_info daylight_bilgi;
void default_dayligth_params(void);
Daylight_info read_daylight(uint32_t adc_degeri);
*/
#endif /* ELIB_EDAYLIGHT_H_ */
