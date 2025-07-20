/*
 * eled.h
 *
 *  Created on: Mar 2, 2023
 *      Author: karataym
 */

#ifndef ELIB_ELED_H_
#define ELIB_ELED_H_


#include "stm32f3xx.h"
#include <stdint.h>
extern RTC_HandleTypeDef hrtc;
typedef enum { // TimCodeType
    ELEC_ANODE  = 0,
	ELEC_CATHODE
} AnodeCathodeType;

extern uint8_t alarm_led_status, warn_led_status, buzzer_led_status;
extern uint16_t check_led_delay_time;

typedef struct { // SetRgb
	uint32_t rgb_r_channel;
	uint32_t rgb_g_channel;
	uint32_t rgb_b_channel;
	AnodeCathodeType rgb_Anode_cathode;
} SetLedRgb;
//extern SetLedRgb eledrgb;
typedef enum ecolor {
	color_Red,
	color_White,
	color_Black,
	color_Blue,
	color_Green,
	color_Yellow,
	color_Lime,
	color_orange,
	color_off,
	color_cyan,
	color_magenta
} E_color;

void eled_init(TIM_HandleTypeDef *htim, uint32_t rgb_r, uint32_t rgb_g, uint32_t rgb_b, AnodeCathodeType code);
GPIO_PinState eled_status(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pinx);
void eled_on(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pinx);
void eled_off(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pinx);
void eled_check(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pinx);
GPIO_PinState set_led(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pinx, uint8_t on_off);
void set_led_rgb(uint16_t R_percent, uint16_t G_percent, uint16_t B_percent);
void eled_toogle(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pinx);
void test_led(uint8_t test);
void set_color_led(E_color ecolor);
void check_RGB_toggle(E_color odd, E_color even);
void rgb_toogle( E_color odd, E_color even);
void Alarm_led(void);
void Warning_led(void);
void Processing_led(void);

#endif /* ELIB_ELED_H_ */
