/*
 * INT_EEPROM_int.c
 *
 *  Created on: Dec 29, 2024
 *      Author: Mehmet KARATAY
 */

#include "eeprom_int.h"
#include "eprogs.h"
#include <string.h>

eeinit_addresses_t init_eeprom_defaults;

// String işlemleri
static EEEPROM_Info int_eeprom_info;

INTERNAL_EEPROM_Status INT_EEPROM_Internal_Init(void) { // EEPROM (Flash) bilgilerini ayarla
	int_eeprom_info.start_addr = INT_EEPROM_START_ADDR;
	int_eeprom_info.page_size = FLASH_PAGE_SIZE;
	int_eeprom_info.total_size = FLASH_PAGE_SIZE; // Bir sayfa kullanıyoruz
	int_eeprom_info.end_addr = int_eeprom_info.start_addr + int_eeprom_info.total_size;
	return EEPROM_OK;
}
INTERNAL_EEPROM_Status EEPROM_GetInfo(EEEPROM_Info* info) {
    memcpy(info, &int_eeprom_info, sizeof(EEEPROM_Info));
    return EEPROM_OK;
}
INTERNAL_EEPROM_Status INT_EEPROM_ErasePage(uint32_t page_addr) {
    FLASH_EraseInitTypeDef EraseInitStruct;
    uint32_t PAGEError;
    HAL_FLASH_Unlock();
	#if __has_include("stm32f3xx.h")
    EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
    EraseInitStruct.PageAddress = page_addr;
    EraseInitStruct.NbPages = 1;
	#endif
    if(HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError) != HAL_OK) {
        HAL_FLASH_Lock();
        return EEPROM_ERROR_WRITE;
    }
    HAL_FLASH_Lock();
    return EEPROM_OK;
}
INTERNAL_EEPROM_Status INT_EEPROM_Write(uint32_t addr, uint8_t* data, uint32_t len){
    // Adres kontrolü
    if(addr < int_eeprom_info.start_addr || addr + len > int_eeprom_info.end_addr){
        return EEPROM_ERROR_INVALID_ADDR;
    }
    HAL_FLASH_Unlock();
    // 4 byte'lık gruplar halinde yaz
    for(uint32_t i = 0; i < len; i += 4) {
        uint32_t data_word = 0;
        for(uint8_t j = 0; j < 4 && (i + j) < len; j++) {
            data_word |= (data[i + j] << (8 * j));
        }
        if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, addr + i, data_word) != HAL_OK) {
            HAL_FLASH_Lock();
            return EEPROM_ERROR_WRITE;
        }
    }
    HAL_FLASH_Lock();
    return EEPROM_OK;
}
INTERNAL_EEPROM_Status INT_EEPROM_Read(uint32_t addr, uint8_t* data, uint32_t len){
    // Adres kontrolü
    if(addr < int_eeprom_info.start_addr || addr + len > int_eeprom_info.end_addr) {
        return EEPROM_ERROR_INVALID_ADDR;
    }
    // Doğrudan belleği oku
    memcpy(data, (void*)addr, len);
    return EEPROM_OK;
}
uint16_t INT_EEPROM_CalculateChecksum(uint8_t* data, uint32_t len){
    uint16_t checksum = 0;
    for(uint32_t i = 0; i < len; i++)
    {
        checksum += data[i];
    }
    return checksum;
}
INTERNAL_EEPROM_Status INT_EEPROM_WriteStruct(uint32_t addr, void* data, uint32_t len){
    uint8_t* data_ptr = (uint8_t*)data;
    uint16_t checksum = INT_EEPROM_CalculateChecksum(data_ptr, len - 2);
    // Checksum'ı yapının sonuna ekle
    *((uint16_t*)(data_ptr + len - 2)) = checksum;

    return INT_EEPROM_Write(addr, data_ptr, len);
}
INTERNAL_EEPROM_Status INT_EEPROM_ReadStruct(uint32_t addr, void* data, uint32_t len) {
    INTERNAL_EEPROM_Status status = INT_EEPROM_Read(addr, (uint8_t*)data, len);
    if(status != EEPROM_OK) return status;
    // Checksum kontrolü
    uint8_t* data_ptr = (uint8_t*)data;
    uint16_t stored_checksum = *((uint16_t*)(data_ptr + len - 2));
    uint16_t calculated_checksum = INT_EEPROM_CalculateChecksum(data_ptr, len - 2);
    if(stored_checksum != calculated_checksum)
    {
        return EEPROM_ERROR_READ;
    }
    return EEPROM_OK;
}

INTERNAL_EEPROM_Status INT_EEPROM_WriteString(uint32_t addr, const char* str, uint32_t max_len){
    uint32_t len = strlen(str);
    if(len >= MAX_STRING_LENGTH) {
        len = MAX_STRING_LENGTH - 1;
    }
    if(len >= max_len) {
		len = max_len;
	}
    // Önce string uzunluğunu yaz
    INTERNAL_EEPROM_Status status = INT_EEPROM_Write(addr, (uint8_t*)&len, sizeof(len));
    if(status != EEPROM_OK) return status;
    // Sonra string'i yaz
    return INT_EEPROM_Write(addr + sizeof(len), (uint8_t*)str, len + 1);
}
void UpdateFlashWithErase(uint32_t startAddress, uint32_t endAddress, uint32_t updateAddress, uint32_t newValue) {
    uint32_t backupData[128]; // Yedekleme için geçici bir buffer (sektör boyutuna göre ayarlayın)
    //uint32_t sectorError;
    HAL_StatusTypeDef status;
    //FLASH_EraseInitTypeDef eraseInitStruct;
    // Flash belleği yazma moduna alın
    HAL_FLASH_Unlock();
    // Hedef sektörün verilerini yedekle
    for (uint32_t i = 0; i < (endAddress - startAddress) / 4; i++) {
        backupData[i] = *(__IO uint32_t *)(startAddress + i * 4);
    }
    // Yeni değeri yedeğe ekle
    backupData[(updateAddress - startAddress) / 4] = newValue;
    // Hedef sektörü sil
//    eraseInitStruct.TypeErase = FLASH_TYPEERASE_SECTORS;
//    eraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;
//    eraseInitStruct.Sector = FLASH_SECTOR_6; // Adresinize uygun sektörü seçin
//    eraseInitStruct.NbSectors = 1;
//    status = HAL_FLASHEx_Erase(&eraseInitStruct, &sectorError);
//    if (status != HAL_OK) {
//        // Hata durumunu yönetin
//        HAL_FLASH_Lock();
//        return;
//    }
    // Yedeklenmiş veriyi geri yaz
    for (uint32_t i = 0; i < (endAddress - startAddress) / 4; i++) {
        status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, startAddress + i * 4, backupData[i]);
        if (status != HAL_OK) {
            // Hata durumunu yönetin
            HAL_FLASH_Lock();
            return;
        }
    }
    // Flash belleği kilitle
    HAL_FLASH_Lock();
}

INTERNAL_EEPROM_Status INT_EEPROM_Reset(uint32_t startAddress, uint32_t len){
    uint32_t endAddress = startAddress + len;
    uint8_t value = 0XFF;
	for (uint32_t address = startAddress; address < endAddress; address += 4) {
		INT_EEPROM_Write(address, (uint8_t*)&value, sizeof(value));
	}
    return EEPROM_OK;
}
INTERNAL_EEPROM_Status INT_EEPROM_ReadString(uint32_t addr, char* str, uint32_t max_len){
    uint32_t len;
    // Önce string uzunluğunu oku
    INTERNAL_EEPROM_Status status = INT_EEPROM_Read(addr, (uint8_t*)&len, sizeof(len));
    if(status != EEPROM_OK) return status;
    // Uzunluk kontrolü
    if(len >= max_len) {
        len = max_len - 1;
    }
    // String'i oku
    status = INT_EEPROM_Read(addr + sizeof(len), (uint8_t*)str, len + 1);
    if(status != EEPROM_OK) return status;
    // String'in sonunu garantile
    str[len] = '\0';
    return EEPROM_OK;
}
// Integer işlemleri
INTERNAL_EEPROM_Status INT_EEPROM_WriteInt8(uint32_t addr, int8_t value) {
    return INT_EEPROM_Write(addr, (uint8_t*)&value, sizeof(value));
}
INTERNAL_EEPROM_Status INT_EEPROM_ReadInt8(uint32_t addr, int8_t* value) {
    return INT_EEPROM_Read(addr, (uint8_t*)value, sizeof(*value));
}
INTERNAL_EEPROM_Status INT_EEPROM_WriteInt16(uint32_t addr, int16_t value) {
    return INT_EEPROM_Write(addr, (uint8_t*)&value, sizeof(value));
}
INTERNAL_EEPROM_Status INT_EEPROM_ReadInt16(uint32_t addr, int16_t *value) {
    return INT_EEPROM_Read(addr, (uint8_t*)value, sizeof(*value));
}
INTERNAL_EEPROM_Status INT_EEPROM_WriteInt32(uint32_t addr, int32_t value) {
    return INT_EEPROM_Write(addr, (uint8_t*)&value, sizeof(value));
}
INTERNAL_EEPROM_Status INT_EEPROM_ReadInt32(uint32_t addr, int32_t* value) {
    return INT_EEPROM_Read(addr, (uint8_t*)value, sizeof(*value));
}
// Float işlemleri
INTERNAL_EEPROM_Status INT_EEPROM_WriteFloat(uint32_t addr, float value) {
    return INT_EEPROM_Write(addr, (uint8_t*)&value, sizeof(value));
}
INTERNAL_EEPROM_Status INT_EEPROM_ReadFloat(uint32_t addr, float* value) {
    return INT_EEPROM_Read(addr, (uint8_t*)value, sizeof(*value));
}
INTERNAL_EEPROM_Status INT_EEPROM_WriteDouble(uint32_t addr, double value) {
    return INT_EEPROM_Write(addr, (uint8_t*)&value, sizeof(value));
}
INTERNAL_EEPROM_Status INT_EEPROM_ReadDouble(uint32_t addr, double* value) {
    return INT_EEPROM_Read(addr, (uint8_t*)value, sizeof(*value));
}
void int_eeprom_einit_reset(void) {
}
void int_eeprom_einit_read(void) {
	INT_EEPROM_ReadInt8(EEPROM_CONST_ID, &einit.eeprom_enum_id[CONST_EEPROM]);
	INT_EEPROM_ReadString(ECOMPANYNAME, einit.ebanner.ecompanyname, 20);
	INT_EEPROM_ReadString(ETECHNICAL_PER, einit.ebanner.etechnical_per, 20);
	INT_EEPROM_ReadString(ERESPONSIBLE_PER, einit.ebanner.eresponsible_per, 20);
	INT_EEPROM_ReadInt8(EAUTHORIZED, &einit.eauthorized);
	INT_EEPROM_ReadInt8(ELCD_BANNER, &einit.ebanner.elcd_banner);
	INT_EEPROM_ReadInt16(EBANNER_DELAY, &einit.ebanner.ebanner_delay);
	INT_EEPROM_ReadInt16(EBANNER_PERCENT_1, &einit.ebanner.ebanner_percent[0]);
	INT_EEPROM_ReadInt16(EBANNER_PERCENT_2, &einit.ebanner.ebanner_percent[1]);
	INT_EEPROM_ReadInt16(EBANNER_PERCENT_3, &einit.ebanner.ebanner_percent[2]);
	INT_EEPROM_ReadInt32(EPERIOD_SERIAL_SECOND, &einit.eperiod_serial_second);
	INT_EEPROM_ReadInt32(EPERIOD_BLUETOOTH_SECOND, &einit.eperiod_bluetooth_second);
	INT_EEPROM_ReadInt16(ESENSOR_INIT_SECOND, &einit.esensor_init_second);
	INT_EEPROM_ReadInt8(EEPROM_CONFIG_ID, &einit.eeprom_enum_id[CONGIG_EEPROM_0]);

	INT_EEPROM_ReadInt32(MODBUS_BAUDRATE, &einit.emodbus[0].ebaudRate);
	INT_EEPROM_ReadInt16(MODBUS_SLAVEID, &einit.emodbus[0].Slave_ID);
	INT_EEPROM_ReadString(MODBUS_PARITY, &einit.emodbus[0].eparity, 1);
	INT_EEPROM_ReadInt8(MODBUS_DATABITS, &einit.emodbus[0].edataBits);
	INT_EEPROM_ReadInt16(MODBUS_TIMEOUT, &einit.emodbus[0].etimeout);
	INT_EEPROM_ReadInt16(MODBUS_RETRYCOUNT, &einit.emodbus[0].eportNumber);

	INT_EEPROM_ReadString(MODBUS_IPADDRESS, einit.emodbus[0].eipAddress, (uint32_t) 16);
}
void int_eeprom_einit_write(void) {
	INT_EEPROM_WriteInt8(EEPROM_CONST_ID, einit.eeprom_enum_id[CONST_EEPROM]);
	INT_EEPROM_WriteString(ECOMPANYNAME, einit.ebanner.ecompanyname, 20);
	INT_EEPROM_WriteString(ETECHNICAL_PER, einit.ebanner.etechnical_per, 20);
	INT_EEPROM_WriteString(ERESPONSIBLE_PER, einit.ebanner.eresponsible_per, 20);
	INT_EEPROM_WriteInt8(EAUTHORIZED, einit.eauthorized);
	INT_EEPROM_WriteInt8(ELCD_BANNER, einit.ebanner.elcd_banner);
	INT_EEPROM_WriteInt16(EBANNER_DELAY, einit.ebanner.ebanner_delay);
	INT_EEPROM_WriteInt16(EBANNER_PERCENT_1, einit.ebanner.ebanner_percent[0]);
	INT_EEPROM_WriteInt16(EBANNER_PERCENT_2, einit.ebanner.ebanner_percent[1]);
	INT_EEPROM_WriteInt16(EBANNER_PERCENT_3, einit.ebanner.ebanner_percent[2]);
	INT_EEPROM_WriteInt32(EPERIOD_SERIAL_SECOND, einit.eperiod_serial_second);
	INT_EEPROM_WriteInt32(EPERIOD_BLUETOOTH_SECOND, einit.eperiod_bluetooth_second);
	INT_EEPROM_WriteInt16(ESENSOR_INIT_SECOND, einit.esensor_init_second);
	INT_EEPROM_WriteInt8(EEPROM_CONFIG_ID, einit.eeprom_enum_id[CONGIG_EEPROM_0]);

	INT_EEPROM_WriteInt32(MODBUS_BAUDRATE, einit.emodbus[0].ebaudRate);
	INT_EEPROM_WriteInt16(MODBUS_SLAVEID, einit.emodbus[0].Slave_ID);
	INT_EEPROM_WriteString(MODBUS_PARITY, &einit.emodbus[0].eparity, 1);
	INT_EEPROM_WriteInt8(MODBUS_DATABITS, einit.emodbus[0].edataBits);
	INT_EEPROM_WriteInt16(MODBUS_TIMEOUT, einit.emodbus[0].etimeout);
	INT_EEPROM_WriteInt16(MODBUS_RETRYCOUNT, einit.emodbus[0].eportNumber);
	INT_EEPROM_WriteString(MODBUS_IPADDRESS, einit.emodbus[0].eipAddress, (uint32_t) 16);

	int_eeprom_einit_read();
}

