/*
 * erelay.c
 *
 *  Created on: May 19, 2022
 *      Author: karataym
 */

#include "erelay.h"



static Relay_Config *relay_list[MAX_RELAYS] = { NULL };
static uint8_t relay_count = 0;
uint8_t relay_flag = 0;

void eRelay_Init(Relay_Config *erelay, GPIO_TypeDef *bport, uint16_t bpin, GPIO_TypeDef *lport, uint16_t lpin, char *bname, uint8_t rid) {
	if (relay_count >= MAX_RELAYS)
		return;
	// Role listesine ekle
	relay_list[relay_count] = erelay;
	erelay->id = rid;
	erelay->gpio = bport;
	erelay->pin = bpin;
	erelay->led_port = lport;
	erelay->led_pin = lpin;
	erelay->toggle_delay = 1000;
	strncpy(erelay->name, bname, MAX_RELAY_NAME_LENGTH);
	relay_count++;
	// Başlangıç durumu
	erelay->state = RELAY_OFF;
	HAL_GPIO_WritePin(erelay->gpio, erelay->pin, GPIO_PIN_RESET);
	if (erelay->led_port != NULL) {
		HAL_GPIO_WritePin(erelay->led_port, erelay->led_pin, GPIO_PIN_RESET);
	}
}
static Relay_Config* eRelay_Find(uint8_t relay_id) {
	for (uint8_t i = 0; i < relay_count; i++) {
		if (relay_list[i] && relay_list[i]->id == relay_id) {
			return relay_list[i];
		}
	}
	return NULL;
}
void eRelay_SetState(uint8_t relay_id, Relay_State state) {
	Relay_Config *relay = eRelay_Find(relay_id);
	if (!relay)
		return;
	relay->state = state;
	switch (state) {
	case RELAY_ON:
		HAL_GPIO_WritePin(relay->gpio, relay->pin, GPIO_PIN_SET);
		if (relay->led_port) {
			HAL_GPIO_WritePin(relay->led_port, relay->led_pin, GPIO_PIN_SET);
		}
		break;
	case RELAY_OFF:
		HAL_GPIO_WritePin(relay->gpio, relay->pin, GPIO_PIN_RESET);
		if (relay->led_port) {
			HAL_GPIO_WritePin(relay->led_port, relay->led_pin, GPIO_PIN_RESET);
		}
		break;
	case RELAY_TOGGLE:
		// Toggle işlemi Process fonksiyonunda yapılacak
		break;
	}
}
Relay_State eRelay_GetState(uint8_t relay_id) {
	Relay_Config *relay = eRelay_Find(relay_id);
	if (!relay)
		return RELAY_OFF;

	return relay->state;
}
void eRelay_Toggle(uint8_t relay_id) {
	Relay_Config *relay = eRelay_Find(relay_id);
	if (!relay)
		return;
	relay->state = RELAY_TOGGLE;
}
void eRelay_Process(void) {
	static uint32_t last_tick = 0;
	uint32_t current_tick = HAL_GetTick();
	// Her role için kontrol
	for (uint8_t i = 0; i < relay_count; i++) {
		if (!relay_list[i])
			continue;
		// Toggle durumu kontrolü
		if (relay_list[i]->state == RELAY_TOGGLE) {
			if (current_tick - last_tick >= relay_list[i]->toggle_delay) {
				HAL_GPIO_TogglePin(relay_list[i]->gpio, relay_list[i]->pin);
				if (relay_list[i]->led_port) {
					HAL_GPIO_TogglePin(relay_list[i]->led_port,
							relay_list[i]->led_pin);
				}
				last_tick = current_tick;
			}
		}
	}
}
// İsme göre role bulma
Relay_Config* eRelay_FindByName(const char* name) {
    if(!name) return NULL;

    for(uint8_t i = 0; i < relay_count; i++) {
        if(relay_list[i] && strcmp(relay_list[i]->name, name) == 0) {
            return relay_list[i];
        }
    }
    return NULL;
}
// Role ismini alma
const char* eRelay_GetName(uint8_t relay_id) {
    Relay_Config* relay = eRelay_Find(relay_id);
    if(!relay) return "Unknown";

    return relay->name;
}
// Role durumunu kontrol edip ayarlama
void eRelay_Check(uint8_t relay_id, uint8_t status) {
    if(status > 0) {
        eRelay_SetState(relay_id, RELAY_ON);
    } else {
        eRelay_SetState(relay_id, RELAY_OFF);
    }
}
// İsme göre kontrol versiyonu da ekleyelim
void eRelay_CheckByName(const char* relay_name, uint8_t status) {
    Relay_Config* relay = eRelay_FindByName(relay_name);
    if(relay) {
        eRelay_Check(relay->id, status);
    }
}

GPIO_PinState erelay_status(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pinx) {
	return HAL_GPIO_ReadPin(GPIOx, GPIO_Pinx);
}
void erelay_on(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pinx, GPIO_TypeDef *led_gpio,
		uint16_t led_port) {
	HAL_GPIO_WritePin(GPIOx, GPIO_Pinx, GPIO_PIN_SET);
	if (led_gpio != NULL) {
		HAL_GPIO_WritePin(led_gpio, led_port, GPIO_PIN_SET);
	}
}
void erelay_off(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pinx, GPIO_TypeDef *led_gpio,
		uint16_t led_port) {
	HAL_GPIO_WritePin(GPIOx, GPIO_Pinx, GPIO_PIN_RESET);
	if (led_gpio != NULL) {
		HAL_GPIO_WritePin(led_gpio, led_port, GPIO_PIN_RESET);
	}
}
void erelay_check(uint8_t estatus, GPIO_TypeDef *GPIOx, uint16_t GPIO_Pinx,
		GPIO_TypeDef *led_gpio, uint16_t led_port) {
	if (estatus > 0) {
		erelay_on(GPIOx, GPIO_Pinx, led_gpio, led_port);
	} else {
		erelay_off(GPIOx, GPIO_Pinx, led_gpio, led_port);
	}
}
uint32_t erelay_toogle(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pinx,
		GPIO_TypeDef *led_gpio, uint16_t led_port, uint32_t delay_value,
		uint32_t delay_max) {
	if (delay_value < 1) {
		HAL_GPIO_TogglePin(GPIOx, GPIO_Pinx);
		if (erelay_status(GPIOx, GPIO_Pinx) == GPIO_PIN_RESET) {
			erelay_off(GPIOx, GPIO_Pinx, led_gpio, led_port);
		} else {
			erelay_on(GPIOx, GPIO_Pinx, led_gpio, led_port);
		}
		return delay_max;

	} else {
		return delay_value - 1;
	}
}

/*
 *  // Sıcaklığa göre fan kontrolü
    Relay_Check(1, sicaklik > SICAKLIK_ESIK);
    // veya isimle
    Relay_CheckByName("Fan_Kontrol", sicaklik > SICAKLIK_ESIK);
 */
