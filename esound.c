/*
 * eprog.c
 *
 *  Created on: Mar 2, 2023
 *      Author: karataym
 *
 */
#include "esound.h"
#include <string.h>
#include <stdlib.h>

// Buzzer ses frekansları
#define BUZZER_FREQ_ACILIS1    2000    // Açılış melodisi ton 1
#define BUZZER_FREQ_ACILIS2    2500    // Açılış melodisi ton 2
#define BUZZER_FREQ_TAMAM      4000    // Tamamlandı tonu
#define BUZZER_FREQ_IKAZ       3000    // İkaz tonu
#define BUZZER_FREQ_ALARM1     2000    // Alarm ton 1
#define BUZZER_FREQ_ALARM2     3000    // Alarm ton 2

// Buzzer süreleri
#define BUZZER_SURE_ACILIS     5000    // Açılış melodisi toplam süre
#define BUZZER_SURE_TAMAM      3000    // Tamamlandı tonu süresi
#define BUZZER_SURE_IKAZ_ON    500     // İkaz açık kalma süresi
#define BUZZER_SURE_IKAZ_OFF   500     // İkaz kapalı kalma süresi
#define BUZZER_SURE_ALARM_ON   300     // Alarm açık kalma süresi
#define BUZZER_SURE_ALARM_OFF  300     // Alarm kapalı kalma süresi

uint32_t sound_counter = 0;

Buzzer_StateTypeDef buzzer_state = { 0 };
Buzzer_HandleTypeDef buzzer;

// Buzzer konfigürasyonları
static const Buzzer_Konfig BUZZER_KONFIG[] = {
// BUZZER_KAPALI
	{ 0, 0, 0, 0, 0 },
	// BUZZER_ACILIS
	{ BUZZER_FREQ_ACILIS1, BUZZER_FREQ_ACILIS2, 200, 100, BUZZER_SURE_ACILIS },
	// BUZZER_TAMAM
	{ BUZZER_FREQ_TAMAM, 0, BUZZER_SURE_TAMAM, 0, BUZZER_SURE_TAMAM },
	// BUZZER_IKAZ (sürekli)
	{ BUZZER_FREQ_IKAZ, 0, BUZZER_SURE_IKAZ_ON,	BUZZER_SURE_IKAZ_OFF, 0 },
	// BUZZER_ALARM (sürekli)
	{ BUZZER_FREQ_ALARM1, BUZZER_FREQ_ALARM2, BUZZER_SURE_ALARM_ON,	BUZZER_SURE_ALARM_OFF, 0 }
};

static void Buzzer_SetTone(Buzzer_HandleTypeDef buzzer, uint32_t frekans) {
	if (frekans == 0) {
		__HAL_TIM_SET_COMPARE(buzzer.timerHandle, buzzer.timerChannel, 0);
		return;
	}
	uint32_t prescaler = (HAL_RCC_GetPCLK1Freq() / frekans) / 1000;
	__HAL_TIM_SET_PRESCALER(buzzer.timerHandle, prescaler);
	__HAL_TIM_SET_COMPARE(buzzer.timerHandle, buzzer.timerChannel, 500); // %50 duty cycle
}
void Buzzer_Kapat(Buzzer_HandleTypeDef buzzer) {
	buzzer.status.durum = BUZZER_KAPALI;
	buzzer.status.calisiyor = 0;
	Buzzer_SetTone(buzzer, 0);
}
void Buzzer_SetDurum(Buzzer_HandleTypeDef buzzer, Buzzer_Durum durum) {
	buzzer.status.durum = durum;
	buzzer.status.baslama_zamani = HAL_GetTick();
	buzzer.status.son_degisim = buzzer.status.baslama_zamani;
	buzzer.status.aktif_ton = 0;
	buzzer.status.calisiyor = 1;
}
bool Buzzer_IsAktif(Buzzer_HandleTypeDef buzzer) {
	return buzzer.status.calisiyor;
}
void Buzzer_Timer_Callback(Buzzer_HandleTypeDef buzzer) {
	if (!buzzer.status.calisiyor)
		return;
	const Buzzer_Konfig *konfig = &BUZZER_KONFIG[buzzer.status.durum];
	uint32_t simdiki_zaman = HAL_GetTick();
	uint32_t gecen_sure = simdiki_zaman - buzzer.status.son_degisim;
	if (konfig->toplam_sure > 0	&& (buzzer.status.durum == BUZZER_ACILIS || buzzer.status.durum == BUZZER_TAMAM)) {
		if (simdiki_zaman - buzzer.status.baslama_zamani >= konfig->toplam_sure) {
			Buzzer_Kapat(buzzer);
			return;
		}
	}
	// IKAZ ve ALARM durumları için sürekli çalışma
	switch (buzzer.status.durum) {
	case BUZZER_ACILIS:
		if (gecen_sure >= (buzzer.status.aktif_ton ?
						konfig->sure_kapali : konfig->sure_acik)) {
			buzzer.status.aktif_ton = !buzzer.status.aktif_ton;
			buzzer.status.son_degisim = simdiki_zaman;
			Buzzer_SetTone(buzzer, buzzer.status.aktif_ton ? konfig->frekans2 : konfig->frekans1);
		}
		break;
	case BUZZER_IKAZ:
		// Kesikli uyarı - kullanıcı kapatana kadar devam et
		if (gecen_sure >= (buzzer.status.aktif_ton ?	konfig->sure_kapali : konfig->sure_acik)) {
			buzzer.status.aktif_ton = !buzzer.status.aktif_ton;
			buzzer.status.son_degisim = simdiki_zaman;
			Buzzer_SetTone(buzzer, buzzer.status.aktif_ton ? 0 : konfig->frekans1);
		}
		break;
	case BUZZER_ALARM:
		// İki tonlu alarm - kullanıcı kapatana kadar devam et
		if (gecen_sure >= (buzzer.status.aktif_ton ?	konfig->sure_kapali : konfig->sure_acik)) {
			buzzer.status.aktif_ton = !buzzer.status.aktif_ton;
			buzzer.status.son_degisim = simdiki_zaman;
			Buzzer_SetTone(buzzer, buzzer.status.aktif_ton ? konfig->frekans2 : konfig->frekans1);
		}
		break;
	case BUZZER_TAMAM:
		if (!buzzer.status.aktif_ton) {
			buzzer.status.aktif_ton = 1;
			Buzzer_SetTone(buzzer, konfig->frekans1);
		}
		break;
	default:
		Buzzer_Kapat(buzzer);
		break;
	}
}
void UyariVer(Buzzer_HandleTypeDef buzzer) {
	// Kesikli uyarı başlat
	Buzzer_SetDurum(buzzer, BUZZER_IKAZ);
}
void AlarmVer(Buzzer_HandleTypeDef buzzer) {
	// İki tonlu alarm başlat
	Buzzer_SetDurum(buzzer, BUZZER_ALARM);
}
void Buzzer_Init(Buzzer_HandleTypeDef buzzer, TIM_HandleTypeDef* tHandle, uint32_t tChannel,  GPIO_TypeDef* lPort, uint16_t lPin, char* bname) {
	buzzer.timerHandle = tHandle;
	buzzer.timerChannel = tChannel;
	buzzer.ledPort = lPort;
	buzzer.ledPin = lPin;
	buzzer.status.calisiyor = 0;
	buzzer.status.baslama_zamani = 0;
	buzzer.status.son_degisim = 0;
	buzzer.status.aktif_ton = 0;
	buzzer.warningLevel = 0;
	buzzer.buzzername = malloc(MAX_BUZZER_NAME_LENGTH * sizeof(char));
	if (buzzer.buzzername == NULL) {
		printf("Bellek tahsisi başarısız oldu.\n");
		exit(1);
	}
	strncpy(buzzer.buzzername, bname, MAX_BUZZER_NAME_LENGTH);
	HAL_TIM_PWM_Start(buzzer.timerHandle, buzzer.timerChannel);
	if (buzzer.ledPort != NULL)
		HAL_GPIO_WritePin(buzzer.ledPort, buzzer.ledPin, GPIO_PIN_RESET);
}
void Buzzer_SetDutyCycle(Buzzer_HandleTypeDef buzzer, uint32_t dutyCycle)
{
    __HAL_TIM_SET_COMPARE(buzzer.timerHandle, buzzer.timerChannel, dutyCycle);
}
void Buzzer_ToggleLED(Buzzer_HandleTypeDef buzzer)
{
    HAL_GPIO_TogglePin(buzzer.ledPort, buzzer.ledPin);
}
void Buzzer_SetWarningLevel(Buzzer_HandleTypeDef buzzer, uint8_t level)
{
	buzzer.warningLevel = level;
}
void Buzzer_Process(Buzzer_HandleTypeDef buzzer)
{
    if (buzzer.warningLevel == 0)
    {
        Buzzer_SetDutyCycle(buzzer, 0); // Buzzer kapalı
        HAL_GPIO_WritePin(buzzer.ledPort, buzzer.ledPin, GPIO_PIN_RESET);
    }
    else if (buzzer.warningLevel == 1)
    {
        static uint8_t toggle = 0;
        if (toggle)
        {
            Buzzer_SetDutyCycle(buzzer, 50); // İkaz sesi
            Buzzer_ToggleLED(buzzer);
        }
        else
        {
            Buzzer_SetDutyCycle(buzzer, 0);
        }
        toggle = !toggle;
    }
    else if (buzzer.warningLevel == 9)
    {
        Buzzer_SetDutyCycle(buzzer, 128); // Alarm sesi
        HAL_GPIO_WritePin(buzzer.ledPort, buzzer.ledPin, GPIO_PIN_SET);
    }
    else
    {
        uint32_t dutyCycle = 128 + (buzzer.warningLevel * 14); // İkaz ve alarm arasında dalgalanma
        Buzzer_SetDutyCycle(buzzer, dutyCycle);
        HAL_GPIO_WritePin(buzzer.ledPort, buzzer.ledPin, GPIO_PIN_SET);
    }
}
