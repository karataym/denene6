/*
 * eusart.c
 *
 *  Created on: Oct 2, 2023
 *      Author: karataym
 */

#include "eusart.h"
#include <main.h>

uint8_t seri_data;
uint8_t seri_data_array[254];
uint8_t seri_index = 0;
uint8_t seri_flag = 0;

uint8_t blue_data;
uint8_t blue_data_array[254];
uint8_t blue_index = 0;
uint8_t blue_flag = 0;
char *usart_dbid = { 0 };
char *usart_func = { 0 };
char *usart_data = { 0 };
uint8_t check_usart_process = 0;
HAL_StatusTypeDef xusart_result;

uint8_t PostData[UBUFFER_SIZE] = { 0 };
uint8_t ModSeriData[UBUFFER_SIZE] = { 0 };
uint16_t serial_counter = 0;
uint16_t bluetooth_counter = 0;
uint8_t bluetooth_connected = 0;

uint32_t date_counter = 0;

uint8_t usart_eid = UINT8_MAX;
uint32_t usart_baudRate = UINT32_MAX;   // Baud hızı (ör. 9600, 19200, 38400)
char usart_parity = 'X';        // Parity ('N' = None, 'E' = Even, 'O' = Odd)
uint8_t usart_dataBits = UINT8_MAX;        // Veri bitleri (genelde 8)
uint8_t usart_stopBits = UINT8_MAX;        // Stop bitleri (1 veya 2)
uint8_t usart_slaveID = UINT8_MAX;         // Slave ID (Cihaz adresi, 1-247)
uint16_t usart_timeout = UINT16_MAX; // Timeout süresi (ms cinsinden, ör. 1000 ms)
uint16_t usart_retryCount = UINT16_MAX;     // Yeniden deneme sayısı (ör. 3)
uint16_t usart_portNumber = UINT16_MAX;   // TCP için port numarası (ör. 502)
char usart_ipAddress[16];    // Modbus TCP için IP adresi (ör. "192.168.0.1")

extern char sData[254];
extern char rData[254];

void usart_uart_reset() {
	usart_eid = UINT8_MAX;
	usart_baudRate = UINT32_MAX;   // Baud hızı (ör. 9600, 19200, 38400)
	usart_parity = 'X';        // Parity ('N' = None, 'E' = Even, 'O' = Odd)
	usart_dataBits = UINT8_MAX;        // Veri bitleri (genelde 8)
	usart_stopBits = UINT8_MAX;        // Stop bitleri (1 veya 2)
	usart_slaveID = UINT8_MAX;         // Slave ID (Cihaz adresi, 1-247)
	usart_timeout = UINT16_MAX; // Timeout süresi (ms cinsinden, ör. 1000 ms)
	usart_retryCount = UINT16_MAX;     // Yeniden deneme sayısı (ör. 3)
	usart_portNumber = UINT16_MAX;   // TCP için port numarası (ör. 502)
	usart_ipAddress[0] = '\0';  // Modbus TCP için IP adresi (ör. "192.168.0.1")
}

void serial_reset() {
	seri_index = 0;
	seri_flag = 0;
	memset(seri_data_array, '\0', sizeof(seri_data_array));
}
void bluetooth_reset() {
	blue_index = 0;
	blue_flag = 0;
	memset(blue_data_array, '\0', sizeof(blue_data_array));
}
/*
 void sData_to_revData(void) {
 seri_recv_buf_size = strlen(sData);
 //uint8_t x = 0;
 for (uint16_t i = 0; i < BUFFER_SIZE; i++) {
 if (i <= seri_recv_buf_size) {
 seri_recvBuf[i] = sData[i];
 } else {
 seri_recvBuf[i] = '\0';
 }
 }
 memset(sData, 0, sizeof(sData));
 }
 */
HAL_StatusTypeDef eSend_Transmit(UART_HandleTypeDef *huartx) {
	if (einit.mod_seri_bt != 1 && huartx == einit.ebluetooth.handle) {
		memset(sData, '\0', sizeof(sData));
		return HAL_ERROR;
	}
	char invalue[UBUFFER_SIZE] = { 0 };
	strcpy(invalue, sData);
	memset(PostData, 0, sizeof(PostData));
	strcpy(sData, einit.eusart_msg);
	xusart_result = HAL_ERROR;
	if (einit.eauthorized == 1 ) {
		strcat(sData, invalue);
	} else {
//		if (find_str_in_str(sData, const_recv_config) > 0) {
//			strcat(sData, invalue);
//		} else if (find_str_in_str(sData, const_recv_close) > 0) {
//			strcat(sData, invalue);
//		} else if (find_str_in_str(sData, const_read_config) > 0) {
//			strcat(sData, invalue);
//		} else {
			strcat(sData, "{Alert:Hardware not authorized ? }");
			char buf[40] = { 0 };
			sprintf(buf, "%016lu %u \n", date_counter, prg_count);
			strcat(sData, buf);
//		}
	}
	uint8_t b = 0;
	for (uint16_t i = 0; i < strlen(einit.ehardware.version.ebarcode); i++) {
		PostData[i] = einit.ehardware.version.ebarcode[i];
		b = i;
	}
	PostData[b] = ' ';
	b++;
	PostData[b] = const_delimeter;
	b++;
	PostData[b] = ' ';
	b++;
	uint8_t l = b;
	for (uint16_t i = 0; i < strlen(currentDateTimeStr); i++) {
		PostData[l] = currentDateTimeStr[i];
		l++;
	}
	PostData[l] = ' ';
	l++;
	PostData[l] = const_delimeter;
	l++;
	PostData[l] = ' ';
	l++;
	for (uint16_t i = 0; i < (UBUFFER_SIZE - l); i++) {
		PostData[l + i] = sData[i];
	}
	if (einit.mod_seri_bt == 1) {
		memcpy(ModSeriData, PostData, UBUFFER_SIZE);
		// modbus serial bilgisini gönderecek bilgisi
	}
	uint16_t s = strlen((char*) PostData);
	if (huartx == einit.ebluetooth.handle) {
		//HAL_GPIO_WritePin(BT_LED_GPIO_Port, BT_LED_Pin, GPIO_PIN_SET);
	}
	if (huartx == einit.eserial.handle) {
		//HAL_GPIO_WritePin(SERIAL_LED_GPIO_Port, SERIAL_LED_Pin, GPIO_PIN_SET);
	}

	xusart_result = HAL_UART_Transmit(huartx, PostData, s, 100);

	if (huartx == einit.ebluetooth.handle) {
		//HAL_GPIO_WritePin(BT_LED_GPIO_Port, BT_LED_Pin, GPIO_PIN_RESET);
	}
	if (huartx == einit.eserial.handle) {
		//HAL_GPIO_WritePin(SERIAL_LED_GPIO_Port, SERIAL_LED_Pin, GPIO_PIN_RESET);
	}
	memset(sData, '\0', sizeof(sData));
	return xusart_result;
}

