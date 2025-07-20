/*
 * emodbus_slave.h
 *
 *  Created on: May 30, 2024
 *      Author: mehme
 */

#ifndef ELIB_EMODBUS_SLAVE_H_
#define ELIB_EMODBUS_SLAVE_H_

#if __has_include("mb.h")
#include "stm32f3xx.h"
#include "eprogs.h"
#include "eusart.h"

#include "mb.h"
//#include "es_mb.h"


extern uint16_t PROG_Modbus_counter;
extern uint16_t modbus_counter;
extern eMBErrorCode mb_error_status;
extern eMBErrorCode mb_connected_status;

extern uint8_t usart3_tx_data_buff[1];
extern uint8_t usart3_rx_data_buff[1];

#define EREG_INPUT_START   0
#define EREG_INPUT_NREGS   35
#define EREG_HOLDING_START 0
#define EREG_HOLDING_NREGS 35

extern volatile USHORT usRegInputStart;
extern volatile USHORT usRegInputBuf[EREG_INPUT_NREGS];
extern volatile USHORT usRegHoldingStart;
extern volatile USHORT usRegHoldingBuf[EREG_HOLDING_NREGS];
extern volatile uint32_t TimingDelay;
typedef enum {
	FIRST_MODBUS,
	SECOND_MODBUS
} eMODBUS_COUNT_ENUM;

void emodbus_init(eMODBUS_COUNT_ENUM m);
void modbus_check(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);
void modbus_add_counter(void);
void reset_usRegHoldingBuf(uint8_t _start, uint8_t _stop );
float modbus_get_float_abcd(const uint16_t *src);
float modbus_get_float_dcba(const uint16_t *src);
float modbus_get_float_badc(const uint16_t *src);
float modbus_get_float_cdab(const uint16_t *src);
float modbus_get_float(const uint16_t *src);

void modbus_set_float_abcd(float f, uint16_t *dest);
void modbus_set_float_dcba(float f, uint16_t *dest);
void modbus_set_float_badc(float f, uint16_t *dest);
void modbus_set_float_cdab(float f, uint16_t *dest);
void modbus_set_float(float f, uint16_t *dest);
#endif

#endif /* ELIB_EMODBUS_SLAVE_H_ */
