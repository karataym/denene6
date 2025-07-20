/*
 * eled.c
 *
 *  Created on: Kas 20, 2024
 *      Author: karataym
 */

#include "eled.h"
#include "stdbool.h"

uint8_t alarm_led_status = 0, warn_led_status = 0, buzzer_led_status = 0;
static uint8_t odd_even = 0, init_led = 0;
uint16_t check_led_delay_time = 100;
static TIM_HandleTypeDef *eledDriverTimer;
SetLedRgb eledrgb;

// PWM	A11, A12, PB8, PB9

void eled_init(TIM_HandleTypeDef *htim, uint32_t rgb_r, uint32_t rgb_g, uint32_t rgb_b, AnodeCathodeType code) {
	eledDriverTimer = htim;

	eledrgb.rgb_r_channel = rgb_r;
	eledrgb.rgb_g_channel = rgb_g;
	eledrgb.rgb_b_channel = rgb_b;
	eledrgb.rgb_Anode_cathode = code;
	/*
	 eled_timer.Instance = TIM4;
	 eled_timer.Init.Prescaler = 36000-1;   // 1 kHz PWM frekansı
	 eled_timer.Init.CounterMode = TIM_COUNTERMODE_UP;
	 eled_timer.Init.Period = 1000-1;  // ARR değeri
	 eled_timer.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	 eled_timer.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
	 */
	HAL_TIM_PWM_Start(eledDriverTimer, eledrgb.rgb_r_channel);  //RGB_R_Pin
	HAL_TIM_PWM_Start(eledDriverTimer, eledrgb.rgb_g_channel);  //RGB_G_Pin
	HAL_TIM_PWM_Start(eledDriverTimer, eledrgb.rgb_b_channel);  //RGB_B_Pin
	init_led = 1;
}

GPIO_PinState eled_status(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pinx) {
	return HAL_GPIO_ReadPin(GPIOx, GPIO_Pinx);
}
void eled_on(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pinx) {
	HAL_GPIO_WritePin(GPIOx, GPIO_Pinx, GPIO_PIN_SET);
}
void eled_off(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pinx) {
	HAL_GPIO_WritePin(GPIOx, GPIO_Pinx, GPIO_PIN_RESET);
}
void eled_check(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pinx) {
	eled_on(GPIOx, GPIO_Pinx);
	HAL_Delay(check_led_delay_time);
	eled_off(GPIOx, GPIO_Pinx);
	HAL_Delay(check_led_delay_time);
}
GPIO_PinState set_led(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pinx, uint8_t on_off) {
	if (on_off == true) {
		eled_on(GPIOx, GPIO_Pinx);
	} else {
		eled_off(GPIOx, GPIO_Pinx);
	}
	return eled_status(GPIOx, GPIO_Pinx);
}
void eled_toogle(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pinx) {
	if (GPIOx != NULL) {
		HAL_GPIO_TogglePin(GPIOx, GPIO_Pinx);
	}
}
// set_rgb fonksiyonu - RGB parlaklık değerlerini ayarlamak için
void set_led_rgb(uint16_t R_percent, uint16_t G_percent, uint16_t B_percent) {
	if (R_percent > 100)
		R_percent = 100;
	if (G_percent > 100)
		G_percent = 100;
	if (B_percent > 100)
		B_percent = 100;
	uint16_t max_value = eledDriverTimer->Init.Period+1;
	uint16_t R = R_percent * max_value / 100;
	uint16_t G = G_percent * max_value / 100;
	uint16_t B = B_percent * max_value / 100;

	if (eledrgb.rgb_Anode_cathode == ELEC_ANODE) {
		// ELEC_ANODE
		__HAL_TIM_SET_COMPARE(eledDriverTimer, eledrgb.rgb_r_channel, max_value - R);
		__HAL_TIM_SET_COMPARE(eledDriverTimer, eledrgb.rgb_g_channel, max_value - G);
		__HAL_TIM_SET_COMPARE(eledDriverTimer, eledrgb.rgb_b_channel, max_value - B);
	} else {
		// cathode (ters çevirilmiş)
		__HAL_TIM_SET_COMPARE(eledDriverTimer, eledrgb.rgb_r_channel, R); // Kırmızı PWM
		__HAL_TIM_SET_COMPARE(eledDriverTimer, eledrgb.rgb_g_channel, G); // Yeşil PWM
		__HAL_TIM_SET_COMPARE(eledDriverTimer, eledrgb.rgb_b_channel, B);  // Mavi PWM
	}
}

void set_color_led(E_color ecolor) {
	switch (ecolor) {
		case color_Red:
			set_led_rgb(100, 0, 0);  // %100 Kırmızı, %0 Yeşil, %0 Mavi
			break;
		case color_Green:
			set_led_rgb(0, 100, 0);  // %0 Kırmızı, %100 Yeşil, %0 Mavi
			break;
		case color_Blue:
			set_led_rgb(0, 0, 100);  // %0 Kırmızı, %0 Yeşil, %100 Mavi
			break;
		case color_Yellow:
			set_led_rgb(100, 100, 0);  // %100 Kırmızı, %100 Yeşil, %0 Mavi
			break;
		case color_White:
			set_led_rgb(100, 100, 100);  // %100 Kırmızı, %100 Yeşil, %100 Mavi
			break;
		case color_Black:
			set_led_rgb(0, 0, 0);  // Siyah, tüm renkler kapalı
			break;
		case color_Lime:
			set_led_rgb(50, 100, 0);  // Açık yeşil (lime), Kırmızı orta, Yeşil yüksek
			break;
		case color_orange:
			set_led_rgb(100, 50, 0);  // Turuncu, Kırmızı yüksek, Yeşil orta
			break;
		case color_off:
			set_led_rgb(0, 0, 0);  // Tüm LED'ler kapalı
			break;
		case color_cyan:
			set_led_rgb(0, 100, 100);  // %0 Kırmızı, %100 Yeşil, %100 Mavi
			break;
		case color_magenta:
			set_led_rgb(100, 0, 100);  // %100 Kırmızı, %0 Yeşil, %100 Mavi
			break;
		default:
			set_led_rgb(0, 0, 0);  // Tüm LED'ler kapalı
			break;
	}
}
void test_led(uint8_t test) {
	uint16_t x = 500;
	if (test == 1) {
		set_color_led(color_Red);
		HAL_Delay(x);
		set_color_led(color_Green);
		HAL_Delay(x);
		set_color_led(color_Blue);
		HAL_Delay(x);
		set_color_led(color_off);
		HAL_Delay(x);
		set_color_led(color_cyan);
		HAL_Delay(x);
		set_color_led(color_magenta);
		HAL_Delay(x);
		set_color_led(color_White);
		HAL_Delay(x);
		set_color_led(color_orange);
		HAL_Delay(x);
		set_color_led(color_Lime);
		HAL_Delay(x);
		set_color_led(color_Black);
		HAL_Delay(x);
		set_color_led(color_Yellow);
		HAL_Delay(x);
	}
}
void rgb_toogle(E_color odd, E_color even) {
	if (odd_even == true) {
		set_color_led(odd);
	} else {
		set_color_led(even);
	}
	odd_even = !odd_even;
}
void Alarm_led(void) {
	rgb_toogle(color_Red, color_Yellow);
}
void Warning_led(void) {
	rgb_toogle(color_Red, color_Blue);
}
void Buzzer_led(void) {
	rgb_toogle(color_Green, color_White);
}
void App_led(void) {
	rgb_toogle(color_Blue, color_Green);
}
void Processing_led(void) {
	if (init_led == 1) {
		if (alarm_led_status == true) {
			return Alarm_led();
		} else if (warn_led_status == true) {
			return Warning_led();
		} else if (buzzer_led_status == true) {
			return Buzzer_led();
		} else
			App_led();
	}
	return;
}



