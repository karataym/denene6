/*
 * ei2c.h
 *
 *  Created on: May 16, 2025
 *      Author: mehme
 */

#ifndef ELIB_EI2C_H_
#define ELIB_EI2C_H_

#include "stm32f3xx_hal.h"
#include "main.h"
#include <stdio.h>

extern I2C_HandleTypeDef hi2c1; // Kullandığınız I2C arabiriminin handle'ı

void I2C_Tarama(I2C_HandleTypeDef *hi2c, uint8_t i);

#endif /* ELIB_EI2C_H_ */
