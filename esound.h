/*
 * esound.h
 *
 *  Created on: Jan 19, 2023
 *      Author: karataym
 */
#ifndef BUZZER_H
#define BUZZER_H

#include "main.h"
#include "stm32f3xx.h"
#include <stdbool.h>
#include <stdio.h>

#define MAX_BUZZER_NAME_LENGTH 20  // Maksimum role ismi uzunluğu

// Buzzer durumları
typedef enum {
    BUZZER_KAPALI = 0,
    BUZZER_ACILIS,        // Sistem başlangıç melodisi (5 sn)
    BUZZER_TAMAM,         // İşlem tamamlandı (3 sn)
    BUZZER_IKAZ,          // Kesikli uyarı (sürekli)
    BUZZER_ALARM          // İki tonlu alarm (sürekli)
} Buzzer_Durum;

// Buzzer yapılandırması
typedef struct {
    uint32_t frekans1;     // Birinci ton frekansı
    uint32_t frekans2;     // İkinci ton frekansı
    uint32_t sure_acik;    // Açık kalma süresi (ms)
    uint32_t sure_kapali;  // Kapalı kalma süresi (ms)
    uint32_t toplam_sure;  // Toplam çalma süresi (0: sürekli)
} Buzzer_Konfig;

typedef struct {
    Buzzer_Durum durum;
    uint32_t baslama_zamani;
    uint32_t son_degisim;
    uint8_t aktif_ton;
    uint8_t calisiyor;
} Buzzer_StateTypeDef;

typedef struct {
    char* buzzername;
    TIM_HandleTypeDef* timerHandle;
    uint32_t timerChannel;
    GPIO_TypeDef* ledPort;
    uint16_t ledPin;
    uint8_t warningLevel;
    Buzzer_StateTypeDef status;
} Buzzer_HandleTypeDef;

extern uint32_t sound_counter;
//extern Buzzer_HandleTypeDef buzzers;

// Fonksiyon prototipleri
void Buzzer_SetDurum(Buzzer_HandleTypeDef buzzer, Buzzer_Durum durum); // Buzzer durumunu ayarla
void Buzzer_Kapat(Buzzer_HandleTypeDef buzzer);                   // Buzzer'ı kapat
void Buzzer_Timer_Callback(Buzzer_HandleTypeDef buzzer);          // Timer kesme rutini
bool Buzzer_IsAktif(Buzzer_HandleTypeDef buzzer);                 // Buzzer aktif mi?
void UyariVer(Buzzer_HandleTypeDef buzzer);
void AlarmVer(Buzzer_HandleTypeDef buzzer);

void Buzzer_Init(Buzzer_HandleTypeDef buzzer, TIM_HandleTypeDef* tHandle, uint32_t tChannel,  GPIO_TypeDef* lPort, uint16_t lPin, char* bname);
void Buzzer_SetDutyCycle(Buzzer_HandleTypeDef buzzer, uint32_t dutyCycle);
void Buzzer_ToggleLED(Buzzer_HandleTypeDef buzzer);
void Buzzer_SetWarningLevel(Buzzer_HandleTypeDef buzzer, uint8_t level);
void Buzzer_Process(Buzzer_HandleTypeDef buzzer);

#endif

/*
 * // Ana program içinde başlatma
void SystemInit(void) {
    // ... diğer başlatmalar ...

    // Buzzer başlat
    Buzzer_Init();

    // Açılış melodisi çal (5 saniye sonra otomatik kapanır)
    Buzzer_SetDurum(BUZZER_ACILIS);
}

// İşlem tamamlandığında
void IslemTamam(void) {
    // Tamamlandı sesi çal (3 saniye sonra otomatik kapanır)
    Buzzer_SetDurum(BUZZER_TAMAM);
}
// Uyarı durumunda
void UyariVer(void) {
    // Kesikli uyarı başlat (kullanıcı kapatana kadar devam eder)
    Buzzer_SetDurum(BUZZER_IKAZ);
    // ... uyarı işlemleri ...
    // Kullanıcı onayladığında
    if(kullanici_onay) {
        Buzzer_Kapat();
    }
}
// Alarm durumunda
void AlarmVer(void) {
    // İki tonlu alarm başlat (kullanıcı kapatana kadar devam eder)
    Buzzer_SetDurum(BUZZER_ALARM);
    // ... alarm işlemleri ...
    // Kullanıcı onayladığında
    if(kullanici_onay) {
        Buzzer_Kapat();
    }
}
// Timer kesme rutininde
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    if(htim->Instance == TIM4) {  // 1ms timer
        Buzzer_Timer_Callback();
    }
}
// Buzzer durumunu kontrol
void BuzzerKontrol(void) {
    if(Buzzer_IsAktif()) {
        // Buzzer çalışıyor
        // ... gerekli işlemler ...
    }
}
 */
