/*
 * eeprom.h
 *
 *  Created on: 17 Mar 2023
 *      Author: karataym
 */

#ifndef ELIB_EEPROM_H_
#define ELIB_EEPROM_H_

#include "stm32f3xx.h"
#include "math.h"
#include "string.h"
#include <stdint.h>
#include <stdbool.h>
#include "eeprom_int.h"
/*
#define einit_eeprom_const_id (0)
#define einit_ecompanyname (5)
#define einit_etechnical_per (30)
#define einit_eresponsible_per (55)
#define einit_eauthorized (95)
#define einit_elcd_banner (100)
#define einit_ebanner_delay (105)
#define einit_ebanner_percent_1 (110)
#define einit_ebanner_percent_2 (115)
#define einit_ebanner_percent_3 (120)
#define einit_eperiod_serial_second (125)
#define einit_eperiod_bluetooth_second (130)
#define einit_esensor_init_second (135)
#define einit_eeprom_config_id (140)
#define einit_modbus_baudRate (145)
#define einit_modbus_parity (150)
#define einit_modbus_dataBits (155)
#define einit_modbus_stopBits (160)
#define einit_modbus_slaveID (165)
#define einit_modbus_timeout (170)
#define einit_modbus_retryCount (175)
#define einit_modbus_portNumber (180)
#define einit_modbus_ipAddress (185)
#define einit_eeprom_start (250)
*/

typedef enum {
	AT24C01,
	AT24C02,
	AT24C04,
	AT24C08,
	AT24C16,
	AT24C32,
	AT24C64,
	AT24C128,
	AT24C256,
	AT24C512,
	NONE
} EEPROM_models;
typedef struct  {
	I2C_HandleTypeDef *edev_port; /*!< Device's I2C port */
	EEPROM_models edev_model; /*!< Device's model */
	uint16_t edev_addr; /*!< Device's I2C 3 bit address */
	uint16_t edev_ei2c_mem_bit;
	uint16_t edev_pagesize; /*!< Device's page size */
	uint16_t edev_epprom_size; /*!< Device's eeprom size */
	uint8_t edev_eeprom_enum_no;
	int16_t edev_capacity;
	HAL_StatusTypeDef edev_connected; /*!< Initialized flag */
} EAT24Cxx_eeprom_t;

typedef struct EAT24Cxx_i2c_line {
	EAT24Cxx_eeprom_t eeproms[8];
	uint8_t connect_dev_count;
} EAT24Cxx_i2c_line_t;

extern uint8_t eeprom_index;
extern uint16_t eeprom_pageid;
//extern uint16_t eeprom_pageid_orj;
//extern uint16_t eeprom_offsetid;
//extern uint16_t eeprom_offsetid_orj;
extern HAL_StatusTypeDef eeprom_hal_status;
extern int32_t eeprom_int_value;
extern uint8_t eeprom_error;
extern uint8_t eeproms_count;
extern uint8_t EEPROM_CONST;
extern uint8_t EEPROM_CONFIG;
extern uint8_t const_eppromid;
extern uint8_t config_eppromid;
extern HAL_StatusTypeDef eeprom_ret;
extern EAT24Cxx_i2c_line_t i2c_eeprom_devices;



/*
void eeprom_einit_read(void);
void eeprom_einit_write(void);
uint8_t Which_eprom(EEPROM_models w);
//void calc_eeprom_position( uint16_t v, uint8_t pagesize );
//uint16_t GET_pg_bitsize ( uint8_t w );
//uint16_t GET_PAGE_SIZE  ( uint8_t w );
//uint16_t GET_PAGE_COUNT ( uint8_t w );
HAL_StatusTypeDef check_eeprom_connect(uint8_t ind);
void eeproms_create(I2C_HandleTypeDef *i2c_handle);
HAL_StatusTypeDef check_eeprom_conneced(uint8_t ind);
uint8_t set_epprom_params(uint8_t ind, EEPROM_models xmodel, uint16_t epprom_dev_addr);
char * read_str_eeprom(uint8_t xpos, uint8_t xsize);
int32_t read_int16_eeprom(uint8_t xpos);
int32_t read_int16_eeprom_config(uint8_t xpos, uint32_t default_value);
void eeprom_echeck_init(uint8_t id, uint8_t ind);
char* write_str_eeprom(uint8_t xpos, const char* xstr, const uint8_t xsize);
void write_int16_eeprom(uint8_t xpos, uint16_t data);
//void EEPROM_RW (uint8_t RW, uint8_t *data, uint16_t size );
//void  EEPROM_Read_int ( EAT24Cxx_i2c_line_t *eedevice );
//void EEPROM_Read_int_Init ( EAT24Cxx_i2c_line_t *eedevice, uint16_t initvalue );
//int8_t EEPROM_Read_byte ( EAT24Cxx_i2c_line_t *eedevice );
//void EEPROM_Pages_Erase_128 ( EAT24Cxx_i2c_line_t *eedevice, uint8_t page );
//void EEPROM_write_int ( EAT24Cxx_i2c_line_t *eedevice, uint32_t data  );
void EEPROM_write_byte(uint8_t xpos,  uint8_t *data);
*/

uint8_t Which_eprom(EEPROM_models w);
void eeproms_create(I2C_HandleTypeDef *i2c_handle);
uint8_t set_epprom_params(I2C_HandleTypeDef *i2c_handle, uint8_t ind, EEPROM_models xmodel, uint16_t epprom_dev_addr);
void eeprom_einit_write(void);
void eeprom_einit_read(void);
uint8_t get_edev_eeprom_enum_no(uint8_t ind);


HAL_StatusTypeDef EEPROM_Write_Int8(EAT24Cxx_eeprom_t *eeprom, uint16_t mem_addr, int8_t data);
HAL_StatusTypeDef EEPROM_Read_Int8(EAT24Cxx_eeprom_t *eeprom, uint16_t mem_addr, int8_t *data);
HAL_StatusTypeDef EEPROM_Write_Int16(EAT24Cxx_eeprom_t *eeprom, uint16_t mem_addr, int16_t data);
HAL_StatusTypeDef EEPROM_Read_Int16(EAT24Cxx_eeprom_t *eeprom, uint16_t mem_addr, int16_t *data);
HAL_StatusTypeDef EEPROM_Write_Int32(EAT24Cxx_eeprom_t *eeprom, uint16_t mem_addr, int32_t data);
HAL_StatusTypeDef EEPROM_Read_Int32(EAT24Cxx_eeprom_t *eeprom, uint16_t mem_addr, int32_t *data);
HAL_StatusTypeDef EEPROM_Write_Int64(EAT24Cxx_eeprom_t *eeprom, uint16_t mem_addr, int64_t data);
HAL_StatusTypeDef EEPROM_Read_Int64(EAT24Cxx_eeprom_t *eeprom, uint16_t mem_addr, int64_t *data);
HAL_StatusTypeDef EEPROM_Write_Float(EAT24Cxx_eeprom_t *eeprom, uint16_t mem_addr, float data);
HAL_StatusTypeDef EEPROM_Read_Float(EAT24Cxx_eeprom_t *eeprom, uint16_t mem_addr, float *data);
HAL_StatusTypeDef EEPROM_Write_Double(EAT24Cxx_eeprom_t *eeprom, uint16_t mem_addr, double data);
HAL_StatusTypeDef EEPROM_Read_Double(EAT24Cxx_eeprom_t *eeprom, uint16_t mem_addr, double *data);
HAL_StatusTypeDef EEPROM_Write_String(EAT24Cxx_eeprom_t *eeprom, uint16_t mem_addr, char *data, uint16_t length);
HAL_StatusTypeDef EEPROM_Read_String(EAT24Cxx_eeprom_t *eeprom, uint16_t mem_addr, char *data, uint16_t length);
HAL_StatusTypeDef EEEPROM_Reset(EAT24Cxx_eeprom_t *eeprom, uint16_t startAddress, uint16_t size);

#endif /* ELIB_EEPROM_H_ */
