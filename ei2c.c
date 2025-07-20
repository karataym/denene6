/*
 * ei2c.c
 *
 *  Created on: May 16, 2025
 *      Author: mehme
 */

#include "ei2c.h"

void print_Deviceaddress(uint8_t adres) {
    printf("Adres: DEC=%d, HEX=0x%02X, BIN=", adres, adres);
    for (int i = 7; i >= 0; i--) {
        printf("%d", (adres >> i) & 1);
    }
    printf("\n");
}


void I2C_Tarama(I2C_HandleTypeDef *hi2c, uint8_t i) {
    HAL_StatusTypeDef ret;
    printf("\n");
    printf("hi2c%d Cihaz Taramasi Baslatiliyor...\n", i);
    for (uint8_t adres = 0; adres < 128; adres++) {
        ret = HAL_I2C_IsDeviceReady(hi2c, (uint16_t)(adres << 1), 10, 10);
        if (ret == HAL_OK) {
        	print_Deviceaddress(adres);
        }
    }
    printf("hi2c%d Tarama Tamamlandi.\n", i);
}