/*
 * erelay.h
 *
 *  Created on: May 19, 2022
 *      Author: karataym
 */

#ifndef ELIB_ERELAY_H_
#define ELIB_ERELAY_H_

#include "stm32f3xx.h"
#include <string.h>  // strcmp, strncpy vb. string fonksiyonları için

extern uint8_t relay_flag;

#define MAX_RELAYS 2  // Maksimum role sayısı
#define MAX_RELAY_NAME_LENGTH 20  // Maksimum role ismi uzunluğu

// Role durumları
typedef enum {
    RELAY_OFF = 0,
    RELAY_ON = 1,
    RELAY_TOGGLE = 2
} Relay_State;

// Role yapılandırması
typedef struct {
    uint8_t id;                 // Role numarası
    char name[MAX_RELAY_NAME_LENGTH + 1];    // Role ismi (+1 null terminator için)
    GPIO_TypeDef* gpio;         // Role GPIO portu
    uint16_t pin;              // Role pin numarası
    GPIO_TypeDef* led_port;    // LED GPIO portu (opsiyonel)
    uint16_t led_pin;          // LED pin numarası (opsiyonel)
    uint32_t toggle_delay;     // Toggle gecikme süresi
    Relay_State state;         // Mevcut durum
} Relay_Config;

// Fonksiyon prototipleri
void eRelay_Init(Relay_Config* config, GPIO_TypeDef *bport, uint16_t bpin, GPIO_TypeDef *lport, uint16_t lpin, char *bname, uint8_t rid );
void eRelay_SetState(uint8_t relay_id, Relay_State state);
Relay_State eRelay_GetState(uint8_t relay_id);
void eRelay_Toggle(uint8_t relay_id);
void eRelay_Process(void);  // Timer callback içinde çağrılacak
const char* eRelay_GetName(uint8_t relay_id);  // İsim almak için yeni fonksiyon
Relay_Config* eRelay_FindByName(const char* name);  // İsme göre role bulmak için yeni fonksiyon
void eRelay_Check(uint8_t relay_id, uint8_t status);

GPIO_PinState erelay_status(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pinx);
void erelay_on(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pinx, GPIO_TypeDef *led_gpio, uint16_t led_port);
void erelay_off(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pinx, GPIO_TypeDef *led_gpio, uint16_t led_port);
uint32_t erelay_toogle(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pinx,	GPIO_TypeDef *led_gpio,	uint16_t led_port,
		uint32_t delay, uint32_t delay_max);
void erelay_check(uint8_t estatus, GPIO_TypeDef *GPIOx, uint16_t GPIO_Pinx,
		GPIO_TypeDef *led_gpio, uint16_t led_port);

#endif /* ELIB_ERELAY_H_ */
//  Main dosyasına eklemeyi unutma
//void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
//	if(htim->Instance == einit.ebuzzer_timer->Instance) {
//	   Relay_Process();
//	}
//	/* USER CODE BEGIN PeriodElapsedCallback 0 */
//
//	/* USER CODE END PeriodElapsedCallback 0 */
//}



/*
 * // Role yapılandırmaları
Relay_Config relay1 = {
    .id = 1,
    .gpio = GPIOA,
    .pin = GPIO_PIN_1,
    .led_gpio = GPIOB,
    .led_pin = GPIO_PIN_1,
    .toggle_delay = 1000  // 1 saniye
};

Relay_Config relay2 = {
    .id = 2,
    .gpio = GPIOA,
    .pin = GPIO_PIN_2,
    .led_gpio = NULL,     // LED kullanılmayacak
    .led_pin = 0,
    .toggle_delay = 500   // 500ms
};

void setup(void) {
    // Roleleri başlat
    Relay_Init(&relay1);
    Relay_Init(&relay2);
}

// Timer callback içinde
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    if(htim->Instance == TIM2) {
        Relay_Process();
    }
}

// Kullanım
void example(void) {
    // Role 1'i aç
    Relay_SetState(1, RELAY_ON);
    // Role 2'yi kapat
    Relay_SetState(2, RELAY_OFF);
    // Role 1'i toggle moduna al
    Relay_Toggle(1);
    // Role 1'in durumunu kontrol et
    if(Relay_GetState(1) == RELAY_ON) {
        // Role açık
    }
}
 */
