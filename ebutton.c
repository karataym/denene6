/*
 * ebutton.c
 *
 *  Created on: Oct 2, 2023
 *      Author: karataym
 *
 */

#include "ebutton.h"
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

// Global buton dizisi tanımı

Button_Config ebuttons[EBUTTON_MAX_BUTTONS];
bool button_long_press_active_flags[EBUTTON_MAX_BUTTONS] = {false}; // Başlangıçta false
uint8_t button_count = 0;
uint8_t buzzer_on_off = 0;

TIM_HandleTypeDef *buzzer_tim;
int8_t  buzzer_channel_no = -1;

void eButton_Init( GPIO_TypeDef *bport, uint16_t bpin,	char *bname, uint8_t bid) {
	if (bid >= EBUTTON_MAX_BUTTONS) {
		printf("Uyarı: Maksimum buton sayısına ulaşıldı!\n");
		return;
	}
	ebuttons[bid].name[MAX_BUTTON_NAME_LENGTH] = '\0';
	ebuttons[bid].state = BUTTON_IDLE;
	ebuttons[bid].pressStartTime = 0;
	ebuttons[bid].pressDuration = 0;
	ebuttons[bid].consecutivePressCount = 0;
	ebuttons[bid].buttonReleased = 1;
	strncpy(ebuttons[bid].name, bname, MAX_BUTTON_NAME_LENGTH);
	ebuttons[bid].name[MAX_BUTTON_NAME_LENGTH] = '\0'; // Garanti null sonlandırma
	ebuttons[bid].id = bid;
	ebuttons[bid].port = bport;
	ebuttons[bid].pin = bpin;
	ebuttons[bid].led_port = NULL;
	ebuttons[bid].led_pin = 0;
	ebuttons[bid].lastButtonState = GPIO_PIN_RESET; // Veya butonun varsayılan durumu (eğer pull-up ise SET)
	ebuttons[bid].debouncedButtonState = GPIO_PIN_RESET; // Aynı şekilde
	ebuttons[bid].lastDebounceTime = 0; // Bu zaten var
	ebuttons[bid].longPressFinalizedTime = 0;
	if (ebuttons[bid].config.shortPressDuration == 0)
		ebuttons[bid].config.shortPressDuration = BTN_SHORT_PRESS_DEF;
	if (ebuttons[bid].config.longPressDuration == 0)
		ebuttons[bid].config.longPressDuration = BTN_LONG_PRESS_DEF;
	if (ebuttons[bid].config.buzzerFrequency == 0)
		ebuttons[bid].config.buzzerFrequency = BTN_BUZZER_FREQ;
	if (ebuttons[bid].config.buzzerDuration == 0)
		ebuttons[bid].config.buzzerDuration = BTN_BUZZER_DUR;
	if (ebuttons[bid].button_buzzer.press_frequency == 0)
		ebuttons[bid].button_buzzer.press_frequency = BTN_PRESS_FREQ_DEFAULT;
	if (ebuttons[bid].button_buzzer.press_duration == 0)
		ebuttons[bid].button_buzzer.press_duration = BTN_PRESS_DUR_DEFAULT;
	if (ebuttons[bid].button_buzzer.release_frequency == 0)
		ebuttons[bid].button_buzzer.release_frequency = BTN_RELEASE_FREQ_DEFAULT;
	if (ebuttons[bid].button_buzzer.release_duration == 0)
		ebuttons[bid].button_buzzer.release_duration = BTN_RELEASE_DUR_DEFAULT;
	button_count++;
}

Button_Config* eButton_GetById(uint8_t id) {
    for (uint8_t i = 0; i < button_count; i++) {
        if (ebuttons[i].id == id) {
            return &ebuttons[i];
        }
    }
    return NULL; // ID'ye sahip bir buton bulunamazsa NULL döndür
}

Button_State eButton_GetState(const uint8_t b) {
	Button_Config *button = eButton_GetById(b);
	if (button) {
		return button->state;
	}
	return BUTTON_IDLE; // Bulunamazsa varsayılan bir durum döndür
}

uint32_t eButton_GetPressDurationID(const uint8_t b) {
	Button_Config *button = eButton_GetById(b);
	if (button) {
		return button->pressDuration;
	}
	return 0;
}

void eButton_buzzer(uint32_t frequency) {
	if (buzzer_tim != NULL && buzzer_tim->Instance != NULL) {
		if (buzzer_channel_no == 1)
			buzzer_tim->Instance->CCR1 = frequency;
		else if (buzzer_channel_no == 2)
			buzzer_tim->Instance->CCR2 = frequency;
		else if (buzzer_channel_no == 3)
			buzzer_tim->Instance->CCR3 = frequency;
		else if (buzzer_channel_no == 4)
			buzzer_tim->Instance->CCR4 = frequency;
		else if (buzzer_channel_no == 5)
			buzzer_tim->Instance->CCR5 = frequency;
		else if (buzzer_channel_no == 6)
			buzzer_tim->Instance->CCR6 = frequency;

	}
}

void eButton_buzzer_off(void) {
	if (buzzer_tim != NULL && buzzer_tim->Instance != NULL) {
		eButton_buzzer(0);
	}
}

void eButton_Process(void) {
    uint32_t currentTime = HAL_GetTick();

    for (uint8_t i = 0; i < button_count; i++) {
        Button_Config *ebutton = &ebuttons[i];
        GPIO_PinState currentPinState = HAL_GPIO_ReadPin(ebutton->port, ebutton->pin);

        if (currentPinState != ebutton->lastButtonState) {
            ebutton->lastDebounceTime = currentTime;
        }

        if ((currentTime - ebutton->lastDebounceTime) > 50) { // 50ms debounce süresi
            if (currentPinState != ebutton->debouncedButtonState) {
                ebutton->debouncedButtonState = currentPinState;

                if (ebutton->debouncedButtonState == GPIO_PIN_SET) { // --- Buton Basıldı ---
                    if (ebutton->buttonReleased) {
                        ebutton->buttonReleased = 0;
                        ebutton->pressStartTime = currentTime;
                        // Sadece PRESSED olarak işaretle, diğer state'leri etkileme
                        // Eğer önceki bir olay işlenmeyi bekliyorsa (örn: SHORT_PRESS finalleşmediyse)
                        // yeni bir basış bu durumu etkilememeli.
                        // Ancak, eğer bir tıklama dizisi açıksa ve yeni basış çok geç geliyorsa,
                        // aşağıdaki bırakma mantığı veya nihaleştirme mantığı bunu sıfırlayacaktır.
                        if (ebutton->state != BUTTON_LONG_PRESS) { // Uzun basma devam etmiyorsa
                             ebutton->state = BUTTON_PRESSED;
                        }
                    }
                }
                else {                 // --- Buton Bırakıldı --- // ebutton->debouncedButtonState == GPIO_PIN_RESET
                    if (!ebutton->buttonReleased) {
                        ebutton->buttonReleased = 1;
                        ebutton->pressDuration = currentTime - ebutton->pressStartTime;

                        // Önceki bir tıklama dizisi açıksa ve bu bırakma çok geç geliyorsa, sayacı sıfırla.
                        // Bu, (consecutivePressCount > 0) kontrolünden önce yapılmalı.
                        if (ebutton->consecutivePressCount > 0 && (currentTime - ebutton->lastClickTime) >= BTN_MULTI_CLICK_WINDOW) {
                            // Eğer bir önceki tıklama dizisi zaten zaman aşımına uğradıysa ve işlenmediyse
                            // (ki bu olmamalı, aşağıdaki nihaleştirme bloğu bunu yakalamalı),
                            // o zaman yeni bir başlangıç yapıyoruz.
                            // Aslında bu durum, aşağıdaki "ÇOKLU TIKLAMA NİHAİLEŞTİRME" bloğunun işi.
                            // Burada sadece consecutivePressCount'u sıfırlamak yerine,
                            // BTN_MULTI_CLICK_WINDOW içindeyse artırmaya odaklanalım.
                        }

                        // UZUN BASMA TESPİTİ (Bırakıldığında)
                        if (ebutton->pressDuration > ebutton->config.longPressDuration) { // longPressDuration
                            ebutton->state = BUTTON_LONG_PRESS;
                            ebutton->longPressFinalizedTime = currentTime; // Uzun basmanın bittiği zamanı kaydet
                            ebutton->consecutivePressCount = 0;
                            ebutton->lastClickTime = 0; // Multi-click için bunu sıfırla
                        }
                        // KISA TIKLAMA ADAYI
                        else if (ebutton->pressDuration >= BTN_MIN_CLICK_DURATION &&
                                 ebutton->pressDuration <= ebutton->config.shortPressDuration) {

                            if (ebutton->state == BUTTON_LONG_PRESS) {
                                // Bu durum olmamalı, uzun basma zaten tespit edildi. Ama güvenlik için.
                                ebutton->state = BUTTON_IDLE; // Uzun basmadan sonra kısa basma olmaz.
                                ebutton->consecutivePressCount = 0;
                                ebutton->lastClickTime = 0;
                            } else {
                                // Bir önceki tıklama çoklu tıklama penceresi içindeyse sayacı artır
                                if (ebutton->consecutivePressCount > 0 && (currentTime - ebutton->lastClickTime) < BTN_MULTI_CLICK_WINDOW) {
                                    ebutton->consecutivePressCount++;
                                } else {
                                    // Yeni bir tıklama dizisi başlıyor veya önceki zaman aşımına uğramıştı.
                                    ebutton->consecutivePressCount = 1;
                                }
                                ebutton->lastClickTime = currentTime; // Son *geçerli* tıklamanın bırakılma zamanını kaydet
                                // State'i PRESSED'den IDLE'a çek, çünkü buton bırakıldı ve tıklama sayıldı.
                                // Nihai olay (SHORT, DOUBLE, TRIPLE) daha sonra belirlenecek.
                                if (ebutton->state == BUTTON_PRESSED) {
                                    ebutton->state = BUTTON_IDLE;
                                }
                            }
                        } else { // Geçersiz tıklama süresi
                            if (ebutton->state != BUTTON_LONG_PRESS) {
                                ebutton->state = BUTTON_IDLE;
                            }
                            // Şimdiki mantık: geçersiz tıklama sayacı sıfırlar.
                            if (ebutton->consecutivePressCount > 0) {
                                // Eğer devam eden bir seri varsa ve bu tıklama geçersizse,
                                // o seriyi hemen nihaleştirmek yerine, sayacı sıfırlamak daha temiz olabilir.
                                // Ya da, eğer bu geçersiz tıklama MULTI_CLICK_WINDOW'u aşıyorsa,
                                // önceki seri zaten nihaleştirilmiş olmalı.
                                // Bu "else" bloğu, tıklamanın süresinin geçersiz olduğu anlamına gelir.
                                // Bu durumda, devam eden bir tıklama serisi olmamalı veya bitmeli.
                                // Güvenlik için sayacı ve lastClickTime'ı sıfırlayalım.
                                // Böylece bir sonraki geçerli tıklama yeni bir seri başlatır.
                            }
                            // Eğer bir seri varsa ve bu tıklama geçersizse, o seri sonlanmış demektir.
                            // Aşağıdaki "ÇOKLU TIKLAMA NİHAİLEŞTİRME KONTROLÜ" bu durumu (lastClickTime'a göre) yakalayacaktır.
                            // Burada sadece state'i IDLE yapalım ve sayacı/lastClickTime'ı sıfırlayalım ki
                            // bu geçersiz tıklama bir sonraki olası seriyi etkilemesin.
                            ebutton->consecutivePressCount = 0;
                            ebutton->lastClickTime = 0;
                        }
                    }
                }
            }
        }
        ebutton->lastButtonState = currentPinState;
        // --- Basılı Tutuluyor (SADECE pressDuration'ı GÜNCELLE) ---
        if (!ebutton->buttonReleased && ebutton->debouncedButtonState == GPIO_PIN_SET) {
            ebutton->pressDuration = currentTime - ebutton->pressStartTime;
        }
        // --- ÇOKLU TIKLAMA NİHAİLEŞTİRME KONTROLÜ ---
        if (ebutton->consecutivePressCount > 0 && ebutton->lastClickTime > 0 && /* lastClickTime geçerli olmalı */
            (currentTime - ebutton->lastClickTime) >= BTN_MULTI_CLICK_WINDOW) {
            if (ebutton->state == BUTTON_IDLE || ebutton->state == BUTTON_PRESSED) {
                if (ebutton->consecutivePressCount == 1) {
                    ebutton->state = BUTTON_SHORT_PRESS;
                } else if (ebutton->consecutivePressCount == 2) {
                    ebutton->state = BUTTON_DOUBLE_CLICK;
                } else if (ebutton->consecutivePressCount >= 3) {
                    ebutton->state = BUTTON_TRIPLE_CLICK;
                }
            }
            ebutton->consecutivePressCount = 0;
        }
        // --- State IDLE'a Dönüş (Genel Zaman Aşımı - GÜVENLİK AĞI) ---
        if (ebutton->state != BUTTON_IDLE && ebutton->state != BUTTON_PRESSED) { // IDLE veya anlık PRESSED değilse
            uint32_t event_occurrence_time = 0;
            if (ebutton->state == BUTTON_LONG_PRESS) {
                event_occurrence_time = ebutton->longPressFinalizedTime; // Uzun basmanın bittiği zaman
            } else if (ebutton->state == BUTTON_SHORT_PRESS ||
                       ebutton->state == BUTTON_DOUBLE_CLICK ||
                       ebutton->state == BUTTON_TRIPLE_CLICK) {
                event_occurrence_time = ebutton->lastClickTime; // Çoklu tıklama serisinin son tıklamasının zamanı
            }

            // Eğer olay geçerli bir zamanda olduysa ve üzerinden yeterince süre geçtiyse IDLE yap
            if (event_occurrence_time > 0 && (currentTime - event_occurrence_time) > (BTN_MULTI_CLICK_WINDOW + 700)) { // Daha cömert bir timeout
                // printf("%s: State %d STUCK, forcing to IDLE by timeout\n", ebutton->name, ebutton->state);
                ebutton->state = BUTTON_IDLE;
                ebutton->consecutivePressCount = 0; // Her ihtimale karşı
                ebutton->lastClickTime = 0;
            }
        }
         // Eğer buton bırakılmışsa ve state hala PRESSED ise IDLE yap (bu durum olmamalı ama güvenlik)
        if (ebutton->state == BUTTON_PRESSED && ebutton->buttonReleased) {
            ebutton->state = BUTTON_IDLE;
        }
        if ((ebutton->debouncedButtonState == GPIO_PIN_SET) && buzzer_on_off == 1) {
        	eButton_buzzer(BTN_RELEASE_FREQ_DEFAULT);
        	HAL_Delay(5);
        	eButton_buzzer(0);
        }

        // LED kontrolü
        if (ebutton->led_port) {
            HAL_GPIO_WritePin(ebutton->led_port, ebutton->led_pin,
                              (ebutton->debouncedButtonState == GPIO_PIN_SET) ? GPIO_PIN_SET : GPIO_PIN_RESET);
        }
    } // End for loop
}

void eButton_PlayBuzzer(Button_Config *ebutton, uint8_t is_press) {
	uint16_t frequency =
			is_press ?
					ebutton->button_buzzer.press_frequency :
					ebutton->button_buzzer.release_frequency;
	uint16_t duration =
			is_press ?
					ebutton->button_buzzer.press_duration :
					ebutton->button_buzzer.release_duration;
	eButton_buzzer(frequency);
	HAL_Delay(duration);
	eButton_buzzer_off();
}
