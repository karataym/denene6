/*
 * eprogs.h
 *
 *  Created on: Mar 4, 2025
 *      Author: karatay
 */

#ifndef EPROGS_H_
#define EPROGS_H_

#include "stm32f3xx.h"
#include <string.h>
#include <stdio.h>
#include "eusart.h"
#include "ejson.h"
#include <ctype.h>
#include <stddef.h>

#define const_recv_config "mc_cfg"
#define const_recv_close "mc_close"
#define const_read_config "mc_read"
#define const_delimeter '~'
#define const_ip_address "192.168.1.222"
#define const_port_number 502

extern uint16_t prg_count;
extern uint8_t process_time;
extern char sData[254];


typedef enum { // SetCompaniesType
	EDM = 2, BS4C = 1,
} SetCompaniesType;

typedef struct { // SetVersionType
	uint16_t emajor;
	uint16_t eminor;
	uint16_t eversion;
	uint16_t eupdate;
	char strcode[20];
	char ebarcode[45];
} SetVersionType;

typedef struct { // SetHardWareType
	uint16_t hw_companyid;
    char hw_projectid[7]; // SetSoftWareType da kullanır
	char hw_functionid[5];  // SetSoftWareType da kullanır
	char hw_erelase[4];
	uint32_t hw_mcid_reg[4];
	SetVersionType version;
} SetHardWareType;

typedef struct { // SetSoftWareType
	// software barcode
	uint16_t sw_companyid;
	char sw_erelase[4];
	SetVersionType version;
} SetSoftWareType;

typedef enum { // USART Baudrate Enum
	MODBUS_BAUD_4800 = 4800,
    MODBUS_BAUD_9600 = 9600,
    MODBUS_BAUD_19200 = 19200,
    MODBUS_BAUD_38400 = 38400,
    MODBUS_BAUD_57600 = 57600,
    MODBUS_BAUD_115200 = 115200
} ModbusBaudRateType;

typedef struct { // SetLcdBannerType
	char ecompanyname[21];
	char etechnical_per[21];  // teknik
	char eresponsible_per[21]; // yetkili
	uint64_t ecustomer_no;
	int8_t elcd_banner;
	int16_t ebanner_delay;
	int16_t ebanner_percent[6];
} SetLcdBannerType;

typedef enum { // Genel Module kodları
    MODULE_BURNER  = 0,
	MODULE_IRRIGATION,
	MODULE_FIRE_TYPE,
	MODULE_CAMERA
} ModuleCode;

typedef enum { // Genel Module kodları
    EMODBUS_SLAVE  = 0,
	EMODBUS_MASTER
} ModbusComType;

typedef struct { // SetEusartType
	// connection settings
	uint8_t comm_ID;
	ModbusComType eusart_type; // 1=Master  0=Slave
	ModuleCode eusart_module[8]; // 0=None, 1=Brulor, 2=Sulama
	USART_TypeDef *usart;
	UART_HandleTypeDef *handle;
	int32_t ebaudRate;       // Baud hızı (ör. 9600, 19200, 38400)
	char eparity;            // Parity ('N' = None, 'E' = Even, 'O' = Odd)
	int8_t edataBits;        // Veri bitleri (genelde 8)
	int8_t estopBits;        // Stop bitleri (1 veya 2)
	int16_t Slave_ID;               // Slave ID (Cihaz adresi, 1-247)
	int16_t etimeout;        // Timeout süresi (ms cinsinden, ör. 1000 ms)
	int16_t eretryCount;     // Yeniden deneme sayısı (ör. 3)
	int16_t eportNumber;     // TCP için port numarası (ör. 502)
	char eipAddress[16];      // Modbus TCP için IP adresi (ör. "192.168.0.1")
} SetEusartType;

typedef struct { // SetClockType
	uint8_t e_saniye;
	uint8_t e_dakika;
	uint8_t e_saat;
	uint16_t e_gun;
	uint8_t e_ay;
	uint16_t e_yil;
	uint8_t e_hafta;
	uint8_t ee_saniye;
	uint8_t ee_dakika;
	uint8_t ee_saat;
	RTC_TimeTypeDef eesTime;
	RTC_DateTypeDef eesDate;
} SetClockType;

typedef struct { // SetClocksType
	SetClockType eclock;
	SetClockType ertc_delay_time[4];
	SetClockType elast_alert;
	SetClockType eclock_remiander;
	uint32_t ClockFreq;
} SetClocksType;

typedef enum { // eSlave150Enu
	ePOOL,
	eDMA,
	eIT
} eADCEnum;
typedef struct { // SetAnalogType
#ifdef HAL_ADC_MODULE_ENABLED
	ADC_HandleTypeDef *eanalog_adc;
#endif
	UART_HandleTypeDef *huart;
	eADCEnum e_ADC_type; // 0 Poll  1 DMA  2 IT
	uint32_t e_ADC_CHANNEL[4];
	uint32_t eADC_REGULAR_RANK[4];
	uint32_t adc_Buffer[4];
	uint32_t adc_values[4];
	float adc_percent[4];
	// [R1=27kΩ]--- Vout ---[R2=3.3kΩ]
	float adc_R1_value[4];
	float adc_R2_value[4];
	float adc_voltage[4];
	float adc_ref_voltage;
	uint8_t ADC_BUFFER_SIZE;
	uint16_t ADC_THRESHOLD_LOW;
	uint16_t ADC_THRESHOLD_HIGH;
	uint8_t adc_hysteria;
	float  adc_range;
	uint32_t e_vrefint_adc_data;
	float e_vref_analog_voltage;
	uint8_t adc_resolution;
	HAL_StatusTypeDef adc_status;
	uint8_t e_ADC_read_count_default;
} SetAnalogType;

typedef struct { // default_config_params_type
	float brulor_max_isi;
	uint8_t brulor_season_type;
	int8_t subtract_lux_heat;
	int8_t subtract_week_heat;
	float build_heat;
	uint8_t circulationMotorNo;
} default_config_params_type_progs;

typedef enum { // I2cCodeType
    I2C_LCD  = 0,
	I2C_EEPROM
} I2cCodeType;

typedef enum { // TimCodeType
    TIM_LED  = 0,
	TIM_BUZZER,
	TIM_DHT
} TimCodeType;

typedef enum { // ConfigEepromType
    CONST_EEPROM  = 0,
	CONGIG_EEPROM_0,
	CONGIG_EEPROM_1,
	CONGIG_EEPROM_2,
	CONGIG_EEPROM_3,
	CONGIG_EEPROM_4,
	CONGIG_EEPROM_5,
	CONGIG_EEPROM_6,
	CONGIG_EEPROM_7,
} ConfigEepromType;

typedef enum { // ConfigEepromAdr
    CONST_EEPROM_ADR = 0xA0,
    CONFIG_EEPROM_0_ADR = 0xA2,
	CONGIG_EEPROM_1_ADR = 0xA4,
	CONGIG_EEPROM_2_ADR,
	CONGIG_EEPROM_3_ADR,
	CONGIG_EEPROM_4_ADR,
	CONGIG_EEPROM_5_ADR,
	CONGIG_EEPROM_6_ADR,
	CONGIG_EEPROM_7_ADR,
} ConfigEepromAdr;

typedef struct { // EinitType
	SetSoftWareType esoftware;
	SetHardWareType ehardware;
	SetEusartType eserial;
	SetEusartType ebluetooth;
	SetEusartType emodbus[2];
	char eusart_msg[50];
	uint8_t mod_seri_bt;
	TIM_HandleTypeDef *etim[10];  // 0=PWM 1=LED 2=DHT ...;
#ifdef HAL_I2C_MODULE_ENABLED
	I2C_HandleTypeDef *ei2c[4]; // 0=elcd_ID; 1 =e_eproms; ..
#endif
	//SetAnalogType eanalogs;
	SetLcdBannerType ebanner;
	int8_t eauthorized;
	int32_t eperiod_serial_second;
	int32_t eperiod_bluetooth_second;
	int16_t esensor_init_second;
	int8_t eeprom_enum_id[8];
	HAL_StatusTypeDef eeprom_connected[8];
	uint8_t einit_control;
	SetClocksType clocks;
	float ecalc_tick_second;
	uint16_t elcd_address;
#ifdef HAL_RTC_MODULE_ENABLED
	RTC_HandleTypeDef *e_rtc;
#endif
	uint32_t config_status;
	uint8_t eis_debug;
	//error_log_t prg_error;
	uint8_t prg_reset_count;
	float noon_eat_value;
	uint8_t battery_status;
	default_config_params_type_progs brulor_config_params;
} EinitType;

extern EinitType einit;
HAL_StatusTypeDef check_I2C_connected(I2C_HandleTypeDef *i2c_handle, uint16_t i2c_addr);
void prg_add_process(void);
void e_set_GetSysClockFreq(void);
void eprog_set_config_check(uint8_t delay_minute);
void send_nlg_db_close(const char *dbid, const char *xmsg, const char *xdetail);
void evyos_config_wi_gi(char *xdbid, char *xdata, char *usart_func);
void check_init_config(void);
void eprog_config_init(char *xdbid, char *usart_func);
void evyos_config_wt(char *xdbid, char *xdata, char *usart_func);
uint8_t str_to_const_function(char *in_value);
HAL_StatusTypeDef einit_bluetooth_recv_it(void);
HAL_StatusTypeDef einit_serial_recv_it(void);
void show_serial_ports(void);
void check_modbus_params(int32_t ebaudrate);
HAL_StatusTypeDef UpdateUARTConfig(UART_HandleTypeDef *huart, SetEusartType econfig);
uint16_t estr_to_uint16(char *str, uint16_t xdef);
char* uint8_to_char_string(const uint8_t *data);
void get_licence_response(void);
void set_evyos_config_ri(char *xdbid, char *usart_func);


#endif /* EPROGS_H_ */
