/*
 * ebutton.h
 *  * Name: Button library
 *  Created on: Oct 2, 2023
 *      Author: karataym
 */

#ifndef EBUTTON_H_
#define EBUTTON_H_

#include "stm32f3xx.h"

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

extern uint8_t button_flag;
extern uint8_t buzzer_on_off;

// Kullanıcı tarafından yapılandırılabilir maksimum buton sayısı
#define EBUTTON_MAX_BUTTONS 8
#define MAX_BUTTON_NAME_LENGTH 20

// Varsayılan buton ayarları
#define BTN_BUZZER_FREQ (2000)
#define BTN_BUZZER_DUR (50)
#define BTN_DOUBLE_CLICK_TIMEOUT (300)  // ms
#define BTN_TRIPLE_CLICK_TIMEOUT (500)  // ms

#define BTN_PRESS_FREQ_DEFAULT    200    // 2000 2 kHz - Basma sesi
#define BTN_RELEASE_FREQ_DEFAULT  250    // 2500 2.5 kHz - Bırakma sesi
#define BTN_PRESS_DUR_DEFAULT     50      // 50 ms - Basma süresi
#define BTN_RELEASE_DUR_DEFAULT   30      // 30 ms - Bırakma sesi süresi
// Varsayılan buton ayarları
#define BTN_LONG_PRESS_DEF (400)
#define BTN_SHORT_PRESS_DEF (380)      // Bir tıklamanın maksimum süresi 150 çok hassas 200 dah iyi
#define BTN_MIN_CLICK_DURATION (30)    // Bir tıklamanın minimum süresi (gürültü filtresi, isteğe bağlı)
#define BTN_MULTI_CLICK_WINDOW (400)   // Ardışık tıklamalar arasındaki maksimum süre (eski BTN_DOUBLE_CLICK_TIMEOUT)

/*
 * Geliştiriciler genellikle 50ms ile 300ms arasında değişen buton basma süreleri kullanıyor. Özellikle:
Kısa basma süresi: 50ms - 150ms arası yaygın olarak tercih ediliyor.
Uzun basma süresi: 300ms ve üzeri genellikle uzun basma olarak kabul ediliyor.
Çift tıklama süresi: 250ms - 350ms arasında değişiyor.
Debounce süresi: 10ms - 20ms civarında tutuluyor.
Bazı kütüphaneler, çoklu basma kombinasyonlarını ve debounce işlemlerini daha iyi yönetmek için özel algoritmalar kullanıyor.
Eğer projenizde daha hassas bir buton kontrolü istiyorsanız, bu tür kütüphaneleri inceleyerek kendi sürelerinizi optimize edebilirsiniz.
 */
// Buton durumları
typedef enum {
    BUTTON_IDLE = 0,
    BUTTON_PRESSED,         // Anlık basılma
    BUTTON_RELEASED,        // Buton bırakıldı (genel)
    BUTTON_SHORT_PRESS,     // Bırakıldıktan sonra kısa basma tespiti
	BUTTON_LONG_PRESS,
    BUTTON_LONG_PRESS_START, // Uzun basma eşiği aşıldı, hala basılı
    BUTTON_LONG_PRESS_HOLD,  // Uzun basma devam ediyor (opsiyonel, periyodik tetikleme için)
    BUTTON_LONG_PRESS_END,   // Uzun basmadan sonra buton bırakıldı
    BUTTON_DOUBLE_CLICK,
    BUTTON_TRIPLE_CLICK
} Button_State;
// Buton yapılandırması
typedef struct {
	uint8_t id;                              // Buton ID
	char name[MAX_BUTTON_NAME_LENGTH + 1];   // Buton ismi
	GPIO_TypeDef *port;                      // Buton portu
	uint16_t pin;                            // Buton pini
	GPIO_TypeDef *led_port;                  // LED portu (opsiyonel)
	uint16_t led_pin;                        // LED pini
	Button_State state;                      // Buton durumu
	uint32_t pressStartTime;                 // Basma başlangıç zamanı
	uint32_t pressDuration;                  // Basılı tutma süresi
	uint8_t consecutivePressCount;           // Ardışık basma sayısı
	uint8_t buttonReleased;                  // Bırakılma durumu
	uint32_t lastDebounceTime;
	GPIO_PinState debouncedButtonState; // Kararlı (debounced) buton durumu
	GPIO_PinState lastButtonState;     // Bir önceki döngüdeki ham buton durumu (debouncing için)
	uint32_t longPressFinalizedTime; // Uzun basmanın bittiği (bırakıldığı) zamanı kaydetmek için
	struct {
		uint16_t shortPressDuration;         // Kısa basma süresi (ms)
		uint16_t longPressDuration;          // Uzun basma süresi (ms)
		uint16_t buzzerFrequency;            // Buzzer frekansı
		uint16_t buzzerDuration;             // Buzzer süresi
	} config;
	struct {
		uint16_t press_frequency;     // Basma sesi frekansı
		uint16_t press_duration;      // Basma sesi süresi
		uint16_t release_frequency;   // Bırakma sesi frekansı
		uint16_t release_duration;    // Bırakma sesi süresi
	} button_buzzer;
	uint32_t lastClickTime;     // Son tıklama zamanı
} Button_Config;

// Harici buzzer timer değişkeni bildirimi
extern TIM_HandleTypeDef *buzzer_tim;
extern int8_t  buzzer_channel_no;

// Global buton dizisi bildirimi
extern Button_Config ebuttons[EBUTTON_MAX_BUTTONS];
extern uint8_t button_count;
extern bool button_long_press_active_flags[EBUTTON_MAX_BUTTONS];

// Fonksiyon prototipleri
void eButton_Init( GPIO_TypeDef *bport, uint16_t bpin, char *bname, uint8_t bid);
Button_Config* eButton_GetById(uint8_t id);
void eButton_Process(void);
Button_State eButton_GetState(const uint8_t b);
uint32_t eButton_GetPressDurationID(const uint8_t b);
void eButton_buzzer(uint32_t frequency);
void eButton_buzzer_off(void);
void eButton_PlayBuzzer(Button_Config* ebutton, uint8_t is_press);

#endif /* EBUTTON_H_ */

