/*
 * eeprom_int.h
 *
 *  Created on: Dec 29, 2024
 *      Author: mehme
 */

#ifndef ELIB_EEPROM_INT_H_
#define ELIB_EEPROM_INT_H_

#include "stm32f3xx.h"
#include "main.h"
#include <stdio.h>

typedef enum {
    EEPROM_CONST_ID = 0,
    ECOMPANYNAME = 5,
    ETECHNICAL_PER = 30,
    ERESPONSIBLE_PER = 55,
    EAUTHORIZED = 95,
    ELCD_BANNER = 100,
    EBANNER_DELAY = 105,
    EBANNER_PERCENT_1 = 110,
    EBANNER_PERCENT_2 = 115,
    EBANNER_PERCENT_3 = 120,
    EPERIOD_SERIAL_SECOND = 125,
    EPERIOD_BLUETOOTH_SECOND = 130,
    ESENSOR_INIT_SECOND = 135,
    EEPROM_CONFIG_ID = 140,
    MODBUS_BAUDRATE = 145,
    MODBUS_PARITY = 150,
    MODBUS_DATABITS = 155,
    MODBUS_STOPBITS = 160,
    MODBUS_SLAVEID = 165,
    MODBUS_TIMEOUT = 170,
    MODBUS_RETRYCOUNT = 175,
    MODBUS_PORTNUMBER = 180,
    MODBUS_IPADDRESS = 185,
    EEPROM_START = 250
} eeinit_addresses_t;

extern eeinit_addresses_t init_eeprom_defaults;

// Flash sayfası boyutu (STM32F303 için)
#define FLASH_PAGE_SIZE     0x800    // 2KB
#define FLASH_START_ADDR    0x08000000

// Kullanılacak son flash sayfası
#define INT_EEPROM_START_PAGE   63       // Son sayfa
#define INT_EEPROM_START_ADDR   (FLASH_START_ADDR + (FLASH_PAGE_SIZE * INT_EEPROM_START_PAGE))

#define MAX_STRING_LENGTH 64

// Hata kodları
typedef enum {
    EEPROM_OK = 0,
    EEPROM_ERROR_WRITE,
    EEPROM_ERROR_READ,
    EEPROM_ERROR_INVALID_ADDR
} INTERNAL_EEPROM_Status;

// EEPROM bilgileri
typedef struct {
    uint32_t start_addr;
    uint32_t end_addr;
    uint32_t page_size;
    uint32_t total_size;
} EEEPROM_Info;

// Fonksiyon prototipleri
INTERNAL_EEPROM_Status INT_EEPROM_Internal_Init(void);
INTERNAL_EEPROM_Status INT_EEPROM_GetInfo(EEEPROM_Info* info);
INTERNAL_EEPROM_Status INT_EEPROM_Write(uint32_t addr, uint8_t* data, uint32_t len);
INTERNAL_EEPROM_Status INT_EEPROM_Read(uint32_t addr, uint8_t* data, uint32_t len);
INTERNAL_EEPROM_Status INT_EEPROM_WriteStruct(uint32_t addr, void* data, uint32_t len);
INTERNAL_EEPROM_Status INT_EEPROM_ReadStruct(uint32_t addr, void* data, uint32_t len);
INTERNAL_EEPROM_Status INT_EEPROM_ErasePage(uint32_t page_addr);
uint16_t INT_EEPROM_CalculateChecksum(uint8_t* data, uint32_t len);

// Veri tipleri için özel fonksiyonlar
INTERNAL_EEPROM_Status INT_EEPROM_WriteString(uint32_t addr, const char* str, uint32_t max_len);
INTERNAL_EEPROM_Status INT_EEPROM_ReadString(uint32_t addr, char* str, uint32_t max_len);
INTERNAL_EEPROM_Status INT_EEPROM_WriteInt8(uint32_t addr, int8_t value);
INTERNAL_EEPROM_Status INT_EEPROM_ReadInt8(uint32_t addr, int8_t *value);
INTERNAL_EEPROM_Status INT_EEPROM_WriteInt16(uint32_t addr, int16_t value);
INTERNAL_EEPROM_Status INT_EEPROM_ReadInt16(uint32_t addr, int16_t *value);
INTERNAL_EEPROM_Status INT_EEPROM_WriteInt32(uint32_t addr, int32_t value);
INTERNAL_EEPROM_Status INT_EEPROM_ReadInt32(uint32_t addr, int32_t *value);
INTERNAL_EEPROM_Status INT_EEPROM_WriteFloat(uint32_t addr, float value);
INTERNAL_EEPROM_Status INT_EEPROM_ReadFloat(uint32_t addr, float* value);
INTERNAL_EEPROM_Status INT_EEPROM_WriteDouble(uint32_t addr, double value);
INTERNAL_EEPROM_Status INT_EEPROM_ReadDouble(uint32_t addr, double* value);

void int_eeprom_einit_read(void);
void int_eeprom_einit_write(void);
INTERNAL_EEPROM_Status INT_EEPROM_Reset(uint32_t startAddress, uint32_t len);

#endif /* ELIB_EEPROM_INT_H_ */
