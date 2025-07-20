/*
 * eeprom.c
 *
 *  Created on: 17 Mar 2023
 *      Author: karataym
 */

#include "eeprom.h"
#include "eprogs.h"

uint8_t EEPROM_CONST = 0xA0; // 160
uint8_t EEPROM_CONFIG = 0xA2; // 162
uint16_t eeprom_pageid = 0;
//uint16_t eeprom_pageid_orj;
//uint16_t eeprom_offsetid = 0;
//uint16_t eeprom_offsetid_orj = 0;
HAL_StatusTypeDef eeprom_ret;
uint8_t eeprom_index = 0;
uint8_t eeproms_count = 8;
int32_t eeprom_int_value = 0;
HAL_StatusTypeDef eeprom_hal_status = HAL_ERROR;
uint8_t eeprom_error = 0;

// Define the Page Size and number of pages
//#define PAGE_SIZE 128     // in Bytes
// #define PAGE_NUM  512    // number of pages
//devices
#define AT24C01_CAPACITY_SIZE		0x007F // 127
#define AT24C02_CAPACITY_SIZE		0x00FF // 256     /*24C02, 256 X 8 (2K bits)    8-Byte Page Write Mode */
#define AT24C04_CAPACITY_SIZE		0x01FF // 512     /*24C04, 512 X 8 (4K bits)   16-Byte Page Write Mode */
#define AT24C08_CAPACITY_SIZE		0x03FF // 1024    /*24C08, 1024 X 8 (8K bits)  16-Byte Page Write Mode */
#define AT24C16_CAPACITY_SIZE		0x07FF // 2048    /*24C16, 2048 X 8 (16K bits) 16-Byte Page Write Mode */
#define AT24C32_CAPACITY_SIZE		0x0FFF // 4095    /*24C32, 4096 X 8 (16K bits) 16-Byte Page Write Mode */
#define AT24C64_CAPACITY_SIZE		0x1FFF // 8191    /*24C64, 8192 X 8 (16K bits) 16-Byte Page Write Mode */
#define AT24C128_CAPACITY_SIZE		0x3FFF // 16383   /*AT24C128 (16,384 x 8) 64-Byte Page Write Mode */
#define AT24C256_CAPACITY_SIZE		0x7FFF // 32767   /*AT24C256K (32,768 x 8) 64-Byte Page Write Mode */
#define AT24C512_CAPACITY_SIZE		0xFFFF // 65535   /*AT24C512K (65,536 x 8) 128-Byte Page Write Mode */

//page sizes in bytes
#define AT24C01_PAGE_SIZE		8
#define AT24C02_PAGE_SIZE		8
#define AT24C04_PAGE_SIZE		16
#define AT24C08_PAGE_SIZE		16
#define AT24C16_PAGE_SIZE		16
#define AT24C32_PAGE_SIZE		16
#define AT24C64_PAGE_SIZE		16
#define AT24C128_PAGE_SIZE		64
#define AT24C256_PAGE_SIZE		64
#define AT24C512_PAGE_SIZE		128

uint8_t Which_eprom(EEPROM_models w) {
	if (w == AT24C01) {
		return 1;
	}
	if (w == AT24C02) {
		return 2;
	}
	if (w == AT24C04) {
		return 3;
	}
	if (w == AT24C08) {
		return 4;
	}
	if (w == AT24C16) {
		return 5;
	}
	if (w == AT24C32) {
		return 6;
	}
	if (w == AT24C64) {
		return 7;
	}
	if (w == AT24C128) {
		return 8;
	}
	if (w == AT24C256) {
		return 9;
	}
	if (w == AT24C512) {
		return 10;
	}
	return 0;
}

uint16_t GET_PAGE_COUNT(EEPROM_models edev_model) {
	switch (edev_model) {
	case AT24C01:
		return AT24C01_CAPACITY_SIZE / AT24C01_PAGE_SIZE;
	case AT24C02:
		return AT24C02_CAPACITY_SIZE / AT24C02_PAGE_SIZE;
	case AT24C04:
		return AT24C04_CAPACITY_SIZE / AT24C04_PAGE_SIZE;
	case AT24C08:
		return AT24C08_CAPACITY_SIZE / AT24C08_PAGE_SIZE;
	case AT24C16:
		return AT24C16_CAPACITY_SIZE / AT24C16_PAGE_SIZE;
	case AT24C32:
		return AT24C32_CAPACITY_SIZE / AT24C32_PAGE_SIZE;
	case AT24C64:
		return AT24C64_CAPACITY_SIZE / AT24C64_PAGE_SIZE;
	case AT24C128:
		return AT24C128_CAPACITY_SIZE / AT24C128_PAGE_SIZE;
	case AT24C256:
		return AT24C256_CAPACITY_SIZE / AT24C256_PAGE_SIZE;
	case AT24C512:
		return AT24C512_CAPACITY_SIZE / AT24C512_PAGE_SIZE;
	default:
		return AT24C02_PAGE_SIZE;
	}
}

uint16_t GET_PAGE_SIZE(EEPROM_models edev_model) {
	switch (edev_model) {
	case AT24C01:
		return AT24C01_PAGE_SIZE;
	case AT24C02:
		return AT24C02_PAGE_SIZE;
	case AT24C04:
		return AT24C04_PAGE_SIZE;
	case AT24C08:
		return AT24C08_PAGE_SIZE;
	case AT24C16:
		return AT24C16_PAGE_SIZE;
	case AT24C32:
		return AT24C32_PAGE_SIZE;
	case AT24C64:
		return AT24C64_PAGE_SIZE;
	case AT24C128:
		return AT24C128_PAGE_SIZE;
	case AT24C256:
		return AT24C256_PAGE_SIZE;
	case AT24C512:
		return AT24C512_PAGE_SIZE;
	default:
		return AT24C02_PAGE_SIZE;
	}
}

uint16_t GET_pg_bitsize(EEPROM_models edev_model) {
	switch (edev_model) {
	case AT24C01:
	case AT24C02:
		return I2C_MEMADD_SIZE_8BIT;
	case AT24C04:
	case AT24C08:
	case AT24C16:
	case AT24C32:
	case AT24C64:
	case AT24C128:
	case AT24C256:
	case AT24C512:
		return I2C_MEMADD_SIZE_16BIT;
	default:
		return I2C_MEMADD_SIZE_16BIT;
	}
}

int16_t GET_CAPACITY(EEPROM_models edev_model) {
	switch (edev_model) {
	case AT24C01:
		return AT24C01_CAPACITY_SIZE;
	case AT24C02:
		return AT24C02_CAPACITY_SIZE;
	case AT24C04:
		return AT24C04_CAPACITY_SIZE;
	case AT24C08:
		return AT24C08_CAPACITY_SIZE;
	case AT24C16:
		return AT24C16_CAPACITY_SIZE;
	case AT24C32:
		return AT24C32_CAPACITY_SIZE;
	case AT24C64:
		return AT24C64_CAPACITY_SIZE;
	case AT24C128:
		return AT24C128_CAPACITY_SIZE;
	case AT24C256:
		return AT24C256_CAPACITY_SIZE;
	case AT24C512:
		return AT24C512_CAPACITY_SIZE;
	default:
		return AT24C02_CAPACITY_SIZE;
	}
}

union enumber_parser4 {
	int32_t ivalue;
	float fvalue;
	uint8_t cn_bytes[4];
} epconvert_number4;

union enumber_parser8 {
	int64_t ivalue;
	double_t dvalue;
	uint8_t cn_bytes[8];
} epconvert_number8;

EAT24Cxx_i2c_line_t i2c_eeprom_devices;

void eeproms_create(I2C_HandleTypeDef *i2c_handle) {
	i2c_eeprom_devices.connect_dev_count = 0;

	for (uint8_t i = 0; i < eeproms_count; i++) {
		i2c_eeprom_devices.eeproms[i].edev_port = i2c_handle;
		i2c_eeprom_devices.eeproms[i].edev_connected = HAL_ERROR;
		i2c_eeprom_devices.eeproms[i].edev_model = 0;
		i2c_eeprom_devices.eeproms[i].edev_addr = 0;
		i2c_eeprom_devices.eeproms[i].edev_ei2c_mem_bit = 0;
		i2c_eeprom_devices.eeproms[i].edev_pagesize = 0;
		i2c_eeprom_devices.eeproms[i].edev_epprom_size = 0;
	};
}

uint8_t count_eeprom_connections() {
	i2c_eeprom_devices.connect_dev_count = 0;
	for (uint8_t i = 0; i < eeproms_count; i++) {
		if (i2c_eeprom_devices.eeproms[i].edev_connected == HAL_OK) {
			i2c_eeprom_devices.connect_dev_count++;
		}
	}
	return i2c_eeprom_devices.connect_dev_count;
}
uint8_t set_epprom_params(I2C_HandleTypeDef *i2c_handle, uint8_t ind,
		EEPROM_models xmodel, uint16_t epprom_dev_addr) {
	i2c_eeprom_devices.eeproms[ind].edev_port = i2c_handle;
	i2c_eeprom_devices.eeproms[ind].edev_model = xmodel;
	i2c_eeprom_devices.eeproms[ind].edev_addr = epprom_dev_addr;
	i2c_eeprom_devices.eeproms[ind].edev_ei2c_mem_bit = GET_pg_bitsize(xmodel);
	i2c_eeprom_devices.eeproms[ind].edev_pagesize = GET_PAGE_SIZE(xmodel);
	i2c_eeprom_devices.eeproms[ind].edev_epprom_size = GET_PAGE_COUNT(xmodel);
	i2c_eeprom_devices.eeproms[ind].edev_capacity = GET_CAPACITY(xmodel);
	i2c_eeprom_devices.eeproms[ind].edev_eeprom_enum_no = Which_eprom(xmodel);
	i2c_eeprom_devices.eeproms[ind].edev_connected = check_I2C_connected(i2c_handle, ind);
	return count_eeprom_connections();
}
uint8_t get_edev_eeprom_enum_no(uint8_t ind) {
	return i2c_eeprom_devices.eeproms[ind].edev_eeprom_enum_no;
}

// EEPROM'e veri yazma fonksiyonu (8-bit int olarak)
HAL_StatusTypeDef EEPROM_Write_Int8(EAT24Cxx_eeprom_t *eeprom,
		uint16_t mem_addr, int8_t data) {
	uint8_t buf = (uint8_t) data;
	return HAL_I2C_Mem_Write(eeprom->edev_port, eeprom->edev_addr, mem_addr,
			I2C_MEMADD_SIZE_16BIT, &buf, sizeof(buf), 1000);
}

// EEPROM'den veri okuma fonksiyonu (8-bit int olarak)
HAL_StatusTypeDef EEPROM_Read_Int8(EAT24Cxx_eeprom_t *eeprom, uint16_t mem_addr,
		int8_t *data) {
	uint8_t buf;
	HAL_StatusTypeDef ret = HAL_I2C_Mem_Read(eeprom->edev_port,
			eeprom->edev_addr, mem_addr, I2C_MEMADD_SIZE_16BIT, &buf,
			sizeof(buf), 1000);
	*data = (int8_t) buf;
	return ret;
}

// EEPROM'e veri yazma fonksiyonu (16-bit int olarak)
HAL_StatusTypeDef EEPROM_Write_Int16(EAT24Cxx_eeprom_t *eeprom,
		uint16_t mem_addr, int16_t data) {
	uint8_t buf[2];
	buf[0] = data >> 8;
	buf[1] = data & 0xFF;
	return HAL_I2C_Mem_Write(eeprom->edev_port, eeprom->edev_addr, mem_addr,
			I2C_MEMADD_SIZE_16BIT, buf, sizeof(buf), 1000);
}

// EEPROM'den veri okuma fonksiyonu (16-bit int olarak)
HAL_StatusTypeDef EEPROM_Read_Int16(EAT24Cxx_eeprom_t *eeprom, uint16_t mem_addr, int16_t *data) {
	uint8_t buf[2];
	HAL_StatusTypeDef ret = HAL_I2C_Mem_Read(eeprom->edev_port,
			eeprom->edev_addr, mem_addr, I2C_MEMADD_SIZE_16BIT, buf,
			sizeof(buf), 1000);
	*data = (int16_t) (buf[0] << 8 | buf[1]);
	return ret;
}
// 32-bit int yazma
HAL_StatusTypeDef EEPROM_Write_Int32(EAT24Cxx_eeprom_t *eeprom, uint16_t mem_addr, int32_t data) {
	uint8_t buf[4];
	buf[0] = (data >> 24) & 0xFF;
	buf[1] = (data >> 16) & 0xFF;
	buf[2] = (data >> 8) & 0xFF;
	buf[3] = data & 0xFF;
	return HAL_I2C_Mem_Write(eeprom->edev_port, eeprom->edev_addr, mem_addr,
			I2C_MEMADD_SIZE_16BIT, buf, sizeof(buf), 1000);
}
// 32-bit int okuma
HAL_StatusTypeDef EEPROM_Read_Int32(EAT24Cxx_eeprom_t *eeprom,	uint16_t mem_addr, int32_t *data) {
	uint8_t buf[4];
	HAL_StatusTypeDef ret = HAL_I2C_Mem_Read(eeprom->edev_port,
			eeprom->edev_addr, mem_addr, I2C_MEMADD_SIZE_16BIT, buf,
			sizeof(buf), 1000);
	*data = (int32_t) (buf[0] << 24 | buf[1] << 16 | buf[2] << 8 | buf[3]);
	return ret;
}
// 64-bit int yazma
HAL_StatusTypeDef EEPROM_Write_Int64(EAT24Cxx_eeprom_t *eeprom,
		uint16_t mem_addr, int64_t data) {
	uint8_t buf[8];
	buf[0] = (data >> 56) & 0xFF;
	buf[1] = (data >> 48) & 0xFF;
	buf[2] = (data >> 40) & 0xFF;
	buf[3] = (data >> 32) & 0xFF;
	buf[4] = (data >> 24) & 0xFF;
	buf[5] = (data >> 16) & 0xFF;
	buf[6] = (data >> 8) & 0xFF;
	buf[7] = data & 0xFF;
	return HAL_I2C_Mem_Write(eeprom->edev_port, eeprom->edev_addr, mem_addr,
			I2C_MEMADD_SIZE_16BIT, buf, sizeof(buf), 1000);
}

// 64-bit int okuma
HAL_StatusTypeDef EEPROM_Read_Int64(EAT24Cxx_eeprom_t *eeprom,
		uint16_t mem_addr, int64_t *data) {
	uint8_t buf[8];
	HAL_StatusTypeDef ret = HAL_I2C_Mem_Read(eeprom->edev_port,
			eeprom->edev_addr, mem_addr, I2C_MEMADD_SIZE_16BIT, buf,
			sizeof(buf), 1000);
	*data = ((int64_t) buf[0] << 56) | ((int64_t) buf[1] << 48)
			| ((int64_t) buf[2] << 40) | ((int64_t) buf[3] << 32)
			| ((int64_t) buf[4] << 24) | ((int64_t) buf[5] << 16)
			| ((int64_t) buf[6] << 8) | (int64_t) buf[7];
	return ret;
}
// float yazma
HAL_StatusTypeDef EEPROM_Write_Float(EAT24Cxx_eeprom_t *eeprom,
		uint16_t mem_addr, float data) {
	uint8_t buf[4];
	memcpy(buf, &data, sizeof(float));
	return HAL_I2C_Mem_Write(eeprom->edev_port, eeprom->edev_addr, mem_addr,
			I2C_MEMADD_SIZE_16BIT, buf, sizeof(buf), 1000);
}
// float okuma
HAL_StatusTypeDef EEPROM_Read_Float(EAT24Cxx_eeprom_t *eeprom,
		uint16_t mem_addr, float *data) {
	uint8_t buf[4];
	HAL_StatusTypeDef ret = HAL_I2C_Mem_Read(eeprom->edev_port,
			eeprom->edev_addr, mem_addr, I2C_MEMADD_SIZE_16BIT, buf,
			sizeof(buf), 1000);
	memcpy(data, buf, sizeof(float));
	return ret;
}
// double yazma
HAL_StatusTypeDef EEPROM_Write_Double(EAT24Cxx_eeprom_t *eeprom,
		uint16_t mem_addr, double data) {
	uint8_t buf[8];
	memcpy(buf, &data, sizeof(double));
	return HAL_I2C_Mem_Write(eeprom->edev_port, eeprom->edev_addr, mem_addr,
			I2C_MEMADD_SIZE_16BIT, buf, sizeof(buf), 1000);
}
// double okuma
HAL_StatusTypeDef EEPROM_Read_Double(EAT24Cxx_eeprom_t *eeprom,
		uint16_t mem_addr, double *data) {
	uint8_t buf[8];
	HAL_StatusTypeDef ret = HAL_I2C_Mem_Read(eeprom->edev_port,
			eeprom->edev_addr, mem_addr, I2C_MEMADD_SIZE_16BIT, buf,
			sizeof(buf), 1000);
	memcpy(data, buf, sizeof(double));
	return ret;
}
// HAL_StatusTypeDef HAL_I2C_Mem_Write(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t MemAddress,
// uint16_t MemAddSize, uint8_t *pData, uint16_t Size, uint32_t Timeout)
/**
 * @brief Belirtilen başlangıç adresinden itibaren EEPROM'a veri yazma
 * @param eepromAddress EEPROM I2C adresi (örneğin 0x50 << 1)
 * @param startAddress EEPROM üzerindeki başlangıç adresi
 * @param size Yazılacak veri boyutu (byte cinsinden)
 * @param value Yazılacak veri (örneğin 0xFF)
 */
HAL_StatusTypeDef EEEPROM_Reset(EAT24Cxx_eeprom_t *eeprom, uint16_t startAddress, uint16_t size) {
    HAL_StatusTypeDef status;
    uint8_t data[size]; // Yazılacak veri için buffer
    memset(data, 0XFF, size);
    status = HAL_I2C_Mem_Write(eeprom->edev_port, eeprom->edev_addr, startAddress, I2C_MEMADD_SIZE_8BIT, data, size, HAL_MAX_DELAY);
//    if (status != HAL_OK) {
//        // Hata durumu yönetimi
//        printf("EEPROM yazma hatası! Hata kodu: %d\n", status);
//    } else {
//        printf("EEPROM yazma başarılı! Adres: 0x%X, Boyut: %d\n", startAddress, size);
//    }
    return status;
}

// char dizisi (string) yazma
HAL_StatusTypeDef EEPROM_Write_String(EAT24Cxx_eeprom_t *eeprom,
		uint16_t mem_addr, char *data, uint16_t length) {
	return HAL_I2C_Mem_Write(eeprom->edev_port, eeprom->edev_addr, mem_addr,
			I2C_MEMADD_SIZE_16BIT, (uint8_t*) data, length, 1000);
}
// char dizisi (string) okuma
HAL_StatusTypeDef EEPROM_Read_String(EAT24Cxx_eeprom_t *eeprom,
		uint16_t mem_addr, char *data, uint16_t length) {
	return HAL_I2C_Mem_Read(eeprom->edev_port, eeprom->edev_addr, mem_addr,
			I2C_MEMADD_SIZE_16BIT, (uint8_t*) data, length, 1000);
}

// EEPROM_Read_Int8(EAT24Cxx_eeprom_t *eeprom, uint16_t mem_addr, int8_t *data)
void eeprom_einit_read(void) {
	EEPROM_Read_Int8(&i2c_eeprom_devices.eeproms[CONST_EEPROM],
			EEPROM_CONST_ID, &einit.eeprom_enum_id[CONST_EEPROM]);
	EEPROM_Read_String(&i2c_eeprom_devices.eeproms[CONST_EEPROM],
			ECOMPANYNAME, einit.ebanner.ecompanyname, 20);
	EEPROM_Read_String(&i2c_eeprom_devices.eeproms[CONST_EEPROM],
			ETECHNICAL_PER, einit.ebanner.etechnical_per, 20);
	EEPROM_Read_String(&i2c_eeprom_devices.eeproms[CONST_EEPROM],
			ERESPONSIBLE_PER, einit.ebanner.eresponsible_per, 20);
	EEPROM_Read_Int8(&i2c_eeprom_devices.eeproms[CONST_EEPROM],
			EAUTHORIZED, &einit.eauthorized);
	EEPROM_Read_Int8(&i2c_eeprom_devices.eeproms[CONST_EEPROM],
			ELCD_BANNER, &einit.ebanner.elcd_banner);
	EEPROM_Read_Int16(&i2c_eeprom_devices.eeproms[CONST_EEPROM],
			EBANNER_DELAY, &einit.ebanner.ebanner_delay);
	EEPROM_Read_Int16(&i2c_eeprom_devices.eeproms[CONST_EEPROM],
			EBANNER_PERCENT_1, &einit.ebanner.ebanner_percent[0]);
	EEPROM_Read_Int16(&i2c_eeprom_devices.eeproms[CONST_EEPROM],
			EBANNER_PERCENT_2, &einit.ebanner.ebanner_percent[1]);
	EEPROM_Read_Int16(&i2c_eeprom_devices.eeproms[CONST_EEPROM],
			EBANNER_PERCENT_3, &einit.ebanner.ebanner_percent[2]);
	EEPROM_Read_Int32(&i2c_eeprom_devices.eeproms[CONST_EEPROM], EPERIOD_SERIAL_SECOND, &einit.eperiod_serial_second);
	EEPROM_Read_Int32(&i2c_eeprom_devices.eeproms[CONST_EEPROM], EPERIOD_BLUETOOTH_SECOND, &einit.eperiod_bluetooth_second);
	EEPROM_Read_Int16(&i2c_eeprom_devices.eeproms[CONST_EEPROM],
			ESENSOR_INIT_SECOND, &einit.esensor_init_second);
	EEPROM_Read_Int8(&i2c_eeprom_devices.eeproms[CONGIG_EEPROM_0],
			EEPROM_CONFIG_ID, &einit.eeprom_enum_id[CONGIG_EEPROM_0]);
}
void eeprom_einit_write(void) {
	EEPROM_Write_Int8(&i2c_eeprom_devices.eeproms[CONST_EEPROM],
			EEPROM_CONST_ID, einit.eeprom_enum_id[CONST_EEPROM]);
	EEPROM_Write_String(&i2c_eeprom_devices.eeproms[CONST_EEPROM],
			ECOMPANYNAME, einit.ebanner.ecompanyname, 20);
	EEPROM_Write_String(&i2c_eeprom_devices.eeproms[CONST_EEPROM],
			ETECHNICAL_PER, einit.ebanner.etechnical_per, 20);
	EEPROM_Write_String(&i2c_eeprom_devices.eeproms[CONST_EEPROM],
			ERESPONSIBLE_PER, einit.ebanner.eresponsible_per, 20);
	EEPROM_Write_Int8(&i2c_eeprom_devices.eeproms[CONST_EEPROM],
			EAUTHORIZED, einit.eauthorized);
	EEPROM_Write_Int8(&i2c_eeprom_devices.eeproms[CONST_EEPROM],
			ELCD_BANNER, einit.ebanner.elcd_banner);
	EEPROM_Write_Int16(&i2c_eeprom_devices.eeproms[CONST_EEPROM],
			EBANNER_DELAY, einit.ebanner.ebanner_delay);
	EEPROM_Write_Int16(&i2c_eeprom_devices.eeproms[CONST_EEPROM],
			EBANNER_PERCENT_1, einit.ebanner.ebanner_percent[0]);
	EEPROM_Write_Int16(&i2c_eeprom_devices.eeproms[CONST_EEPROM],
			EBANNER_PERCENT_2, einit.ebanner.ebanner_percent[1]);
	EEPROM_Write_Int16(&i2c_eeprom_devices.eeproms[CONST_EEPROM],
			EBANNER_PERCENT_3, einit.ebanner.ebanner_percent[2]);
	EEPROM_Write_Int32(&i2c_eeprom_devices.eeproms[CONST_EEPROM],
			EPERIOD_SERIAL_SECOND, einit.eperiod_serial_second);
	EEPROM_Write_Int32(&i2c_eeprom_devices.eeproms[CONST_EEPROM],
			EPERIOD_BLUETOOTH_SECOND, einit.eperiod_bluetooth_second);
	EEPROM_Write_Int16(&i2c_eeprom_devices.eeproms[CONST_EEPROM],
			ESENSOR_INIT_SECOND, einit.esensor_init_second);
	EEPROM_Write_Int8(&i2c_eeprom_devices.eeproms[CONGIG_EEPROM_0],
			EEPROM_CONFIG_ID, einit.eeprom_enum_id[CONGIG_EEPROM_0]);
	eeprom_einit_read();
}

