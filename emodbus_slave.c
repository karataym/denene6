/*
 * emodbus_slave.c
 *
 *  Created on: May 30, 2024
 *      Author: mehme
 */

#include "emodbus_slave.h"

#if __has_include("mb.h")

uint16_t modbus_counter = 0;
uint16_t PROG_Modbus_counter = 0;
eMBErrorCode mb_error_status;
eMBErrorCode mb_connected_status;
uint8_t usart3_tx_data_buff[1] = { 0 };
uint8_t usart3_rx_data_buff[1] = { 0 };

#define ELOW_HIGH 4

uint8_t elow_bytes[ELOW_HIGH];
uint8_t ehigh_bytes[ELOW_HIGH];

void reset_usRegHoldingBuf(uint8_t _start, uint8_t _stop ) {
	if (_stop > EREG_HOLDING_NREGS) {
		_stop = EREG_HOLDING_NREGS;
	}
	for (size_t i = _start; i <= _stop; i++) {
		usRegHoldingBuf[i] = 0;
	}
}
// Tüm elemanları 0'a ayarlama
void elow_bytes_reset() {
	for (size_t i = 0; i < ELOW_HIGH; i++) {
		elow_bytes[i] = 0;
		ehigh_bytes[i] = 0;
	}
}
void elow_bytes_show() {
	for (size_t i = 0; i < ELOW_HIGH; i++) {
		printf("elow_bytes[%zu] = %d\n", i, elow_bytes[i]);
		printf("ehigh_bytes[%zu] = %d\n", i, ehigh_bytes[i]);
	}
}

volatile USHORT usRegInputStart = EREG_INPUT_START;
volatile USHORT usRegInputBuf[EREG_INPUT_NREGS];
volatile USHORT usRegHoldingStart = EREG_INPUT_START;
volatile USHORT usRegHoldingBuf[EREG_HOLDING_NREGS];
volatile uint32_t TimingDelay;

void emodbus_init(eMODBUS_COUNT_ENUM m) {
	// USART3 için UART ayarlarını güncelle
	// MBMode eMode, UCHAR ucSlaveAddress, UCHAR ucPort, ULONG ulBaudRate, eMBParity eParity
	mb_connected_status = eMBInit(MB_RTU, einit.emodbus[m].Slave_ID, 1,
			einit.emodbus[m].handle->Init.BaudRate, MB_PAR_NONE);
	mb_error_status = eMBEnable();
	if (mb_connected_status == MB_ENOERR) {
		printf("Modbus connected");
		if (mb_error_status == MB_ENOERR) {
			printf("Modbus MBEnable");
		} else {
			printf("Modbus MBEnable Error : %d", mb_error_status);
		}

	} else {
		printf("Modbus Connect Error : %d", mb_connected_status);
	}
}
void modbus_check(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin) {
	//if (modbus_counter > (einit.ecalc_tick_second * 1)) {
		modbus_counter = 0;
		if (GPIOx != NULL)
			HAL_GPIO_TogglePin(GPIOx, GPIO_Pin);
		mb_error_status = eMBPoll(); /*Modbus poll update in each run*/
		if (mb_error_status != MB_ENOERR) {
			memset(sData, '\0', sizeof(sData));
			sprintf(sData, "{'Msg': {'Modbus_Error':%d, 'Counter':%d } }",
					mb_error_status, prg_count);
			eSend_Transmit(einit.eserial.handle);
		}
	//}
}

/**
 * @brief  Inserts a delay time.
 * @param  nTime: specifies the delay time length, in milliseconds.
 * @retval None
 */
//void Delay(__IO uint32_t nTime) {
//	TimingDelay = nTime;
//	while (TimingDelay != 0);
//}
//
///**
// * @brief  Decrements the TimingDelay variable.
// * @param  None
// * @retval None
// */
//void TimingDelay_Decrement(void) {
//	if (TimingDelay != 0x00) {
//		TimingDelay--;
//	}
//}

/* Get a float from 4 bytes (Modbus) without any conversion (ABCD) */
float modbus_get_float_abcd(const uint16_t *src) {
	float f;
	uint32_t i;
	uint8_t a, b, c, d;
	a = (src[0] >> 8) & 0xFF;
	b = (src[0] >> 0) & 0xFF;
	c = (src[1] >> 8) & 0xFF;
	d = (src[1] >> 0) & 0xFF;
	i = (a << 24) | (b << 16) | (c << 8) | (d << 0);
	memcpy(&f, &i, 4);
	return f;
}
/* Get a float from 4 bytes (Modbus) in inversed format (DCBA) */
float modbus_get_float_dcba(const uint16_t *src) {
	float f;
	uint32_t i;
	uint8_t a, b, c, d;
	a = (src[0] >> 8) & 0xFF;
	b = (src[0] >> 0) & 0xFF;
	c = (src[1] >> 8) & 0xFF;
	d = (src[1] >> 0) & 0xFF;
	i = (d << 24) | (c << 16) | (b << 8) | (a << 0);
	memcpy(&f, &i, 4);
	return f;
}
/* Get a float from 4 bytes (Modbus) with swapped bytes (BADC) */
float modbus_get_float_badc(const uint16_t *src) {
	float f;
	uint32_t i;
	uint8_t a, b, c, d;
	a = (src[0] >> 8) & 0xFF;
	b = (src[0] >> 0) & 0xFF;
	c = (src[1] >> 8) & 0xFF;
	d = (src[1] >> 0) & 0xFF;
	i = (b << 24) | (a << 16) | (d << 8) | (c << 0);
	memcpy(&f, &i, 4);
	return f;
}
/* Get a float from 4 bytes (Modbus) with swapped words (CDAB) */
float modbus_get_float_cdab(const uint16_t *src) {
	float f;
	uint32_t i;
	uint8_t a, b, c, d;
	a = (src[0] >> 8) & 0xFF;
	b = (src[0] >> 0) & 0xFF;
	c = (src[1] >> 8) & 0xFF;
	d = (src[1] >> 0) & 0xFF;
	i = (c << 24) | (d << 16) | (a << 8) | (b << 0);
	memcpy(&f, &i, 4);
	return f;
}
/* DEPRECATED - Get a float from 4 bytes in sort of Modbus format */
float modbus_get_float(const uint16_t *src) {
	float f;
	uint32_t i;
	i = (((uint32_t) src[1]) << 16) + src[0];
	memcpy(&f, &i, sizeof(float));
	return f;
}
/* Set a float to 4 bytes for Modbus w/o any conversion (ABCD) */
void modbus_set_float_abcd(float f, uint16_t *dest) {
	uint32_t i;
	uint8_t *out = (uint8_t*) dest;
	uint8_t a, b, c, d;
	memcpy(&i, &f, sizeof(uint32_t));
	a = (i >> 24) & 0xFF;
	b = (i >> 16) & 0xFF;
	c = (i >> 8) & 0xFF;
	d = (i >> 0) & 0xFF;
	out[0] = a;
	out[1] = b;
	out[2] = c;
	out[3] = d;
}
/* Set a float to 4 bytes for Modbus with byte and word swap conversion (DCBA) */
void modbus_set_float_dcba(float f, uint16_t *dest) {
	uint32_t i;
	uint8_t *out = (uint8_t*) dest;
	uint8_t a, b, c, d;
	memcpy(&i, &f, sizeof(uint32_t));
	a = (i >> 24) & 0xFF;
	b = (i >> 16) & 0xFF;
	c = (i >> 8) & 0xFF;
	d = (i >> 0) & 0xFF;
	out[0] = d;
	out[1] = c;
	out[2] = b;
	out[3] = a;
}
/* Set a float to 4 bytes for Modbus with byte swap conversion (BADC) */
void modbus_set_float_badc(float f, uint16_t *dest) {
	uint32_t i;
	uint8_t *out = (uint8_t*) dest;
	uint8_t a, b, c, d;
	memcpy(&i, &f, sizeof(uint32_t));
	a = (i >> 24) & 0xFF;
	b = (i >> 16) & 0xFF;
	c = (i >> 8) & 0xFF;
	d = (i >> 0) & 0xFF;
	out[0] = b;
	out[1] = a;
	out[2] = d;
	out[3] = c;
}
/* Set a float to 4 bytes for Modbus with word swap conversion (CDAB) */
void modbus_set_float_cdab(float f, uint16_t *dest) {
	uint32_t i;
	uint8_t *out = (uint8_t*) dest;
	uint8_t a, b, c, d;
	memcpy(&i, &f, sizeof(uint32_t));
	a = (i >> 24) & 0xFF;
	b = (i >> 16) & 0xFF;
	c = (i >> 8) & 0xFF;
	d = (i >> 0) & 0xFF;
	out[0] = c;
	out[1] = d;
	out[2] = a;
	out[3] = b;
}
/* DEPRECATED - Set a float to 4 bytes in a sort of Modbus format! */
void modbus_set_float(float f, uint16_t *dest) {
	uint32_t i;
	memcpy(&i, &f, sizeof(uint32_t));
	dest[0] = (uint16_t) i;
	dest[1] = (uint16_t) (i >> 16);
}
#endif

/*
*- emodbus_slave klasörü Proje klasörünün altındaki Drivers klasörüne kopyalanir
	Drivers klasörü altında CMSIS, STM32F3xx_HAL_Driver ve emodbus_slave klasörleri ayrı ayrı görülür.

*- MODBUS için IOC Dosyasından TIM1 kullanıma açılır.
*- TIM1 için aşağıdaki parametreler seçilir.
	MODE:
	Clock Source: Internal Clock

	Prescaler PSC: 1200 - 1
	CounterMode: UP
	CounterPeriod: 1000
	Internal Clock Division: No Division
	Repetition Counter 0

	NVIC Settings:
	TIM1 Capture Compare Interrupt Enabled (digerleri disabled)

*- MODBUS için IOC Dosyasından USART3 kullanıma açılır.
*- USART3 için aşağıdaki parametreler seçilir.
	Mode: Asynchronous
	Baudrate 115200
	Word: 8 bits
	Parity: None
	Stop: 1
	Data Direction: Receive and Transmit
	Over Sampling: 16
	Sinble Sample: Disable

	DMA Settings Tab
	RX DMA Enabled, Default Configs
	TX DMA Enabled, Default Configs

	NVIC Settings Tab:
	DMA1 Channel2 Global Interrupt Enabled
	DMA1 Channel3 Global Interrupt Enabled
	USART3 Global Interrupt Enabled

*- PB8 ve PB9 pinleri USART3 RX TX için seçilir, isim olarak MODBUS_RX ve MODBUS_TX verilir
*- PA12 MODBUS Enable pini için GPIO Output olarak seçilir, isim olarak MODBUS_EN verilir.

*- main.c dosyasına eklenecek satırlar ilgili USER CODE yerlerine dikkat edilerek eklenir.

	* USER CODE BEGIN Includes
	#include "mb.h"

	* USER CODE BEGIN PV *
	#define SLAVE_ID 0x01

	volatile USHORT   usRegInputStart = REG_INPUT_START;
	volatile USHORT   usRegInputBuf[REG_INPUT_NREGS];
	volatile USHORT   usRegHoldingStart = REG_HOLDING_START;
	volatile USHORT   usRegHoldingBuf[REG_HOLDING_NREGS];
	volatile uint32_t TimingDelay;

	void main() {
		....

		* USER CODE BEGIN 2 *
		static uint16_t counter_test = 0;

		eMBInit(MB_RTU, SLAVE_ID, 1, 115200, MB_PAR_NONE);
		eMBEnable();

		while(1){
		....

			* USER CODE BEGIN 3 *
			counter_test++;
			usRegHoldingBuf[2] = counter_test; *change buffer 1 input register modbus to a test counter*
			// HAL_GPIO_TogglePin(BUTTON_LED_GPIO_Port, BUTTON_LED_Pin);
			eMBPoll();  *Modbus poll update in each run*
			HAL_Delay(100);
		}
	}


	* USER CODE BEGIN 4 *
	**
	 * @brief  Inserts a delay time.
	 * @param  nTime: specifies the delay time length, in milliseconds.
	 * @retval None
	 *
	void Delay(__IO uint32_t nTime)
	{
		TimingDelay = nTime;

		while(TimingDelay != 0);
	}


	**
	 * @brief  Decrements the TimingDelay variable.
	 * @param  None
	 * @retval None
	 *
	void TimingDelay_Decrement(void)
	{
		if (TimingDelay != 0x00)
		{
			TimingDelay--;
		}
	}

	* USER CODE END 4 *


*- stm32f3xx_it.c dosyasına eklenecek satırlar ilgili USER CODE yerlerine dikkat edilerek eklenir.

	* USER CODE BEGIN 0 *
	#include "mb.h"
	uint8_t usart3_tx_data_buff[1]={0};
	uint8_t usart3_rx_data_buff[1]={0};

	extern void TimingDelay_Decrement(void);
	extern void prvvTIMERExpiredISR( void );
	extern void prvvUARTTxReadyISR(void);
	extern void prvvUARTRxISR(void);
	* USER CODE END 0 *

	void SysTick_Handler(void)
	{
	  * USER CODE BEGIN SysTick_IRQn 0 *
		TimingDelay_Decrement();

	  * USER CODE END SysTick_IRQn 0 *
	  ....
	}

	* USER CODE BEGIN 1 *
	void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
	{
		if(huart->Instance == USART3)
		{
			prvvUARTRxISR();
			HAL_UART_Receive_IT(&huart3, usart3_rx_data_buff, 1);
		}
	}

	void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
	{
		if(huart->Instance == USART3)
		{
			HAL_GPIO_WritePin(MODBUS_EN_GPIO_Port, MODBUS_EN_Pin, GPIO_PIN_RESET);
			prvvUARTTxReadyISR();
		}
	}

	void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim)
	{
		if(htim->Instance == TIM1)
		{
			prvvTIMERExpiredISR( );
		}
	}
	* USER CODE END 1 *

*- STM32 CubeIDE programında Sol tarafta dosya gezgininde proje ismine sag tıklanir
Gelen menüde Refresh tıklanır, Drivers klasörü altında emodbus_slave klasörü olduğu teyit edilir.

*- Yine Proje ismine sag tıklanır, gelen menüden Build Configurations altında Manage'e tıklanır.
*- Gelen pencerede Debug isimli girdi silinir, sadece Release kalır
*- Drivers altında emodbus_slave klasörüne gidilir.
*- emodbus_slave klasörü altındaki 6 farklı klasör shift ile topluca seçilir.
*- Seçili haldeyken sag tıklayarak gelen menün altlarında Add/Remove Include Path'e tıklanır
*- Gelen Menüde sadece Release olacaktır, Ok diyip çıkılır.
*- emodbus_slave klasörün ismine sag tıklayıp gelen menün altlarında Add/Remove Include Path'e tıklanır
*- Gelen Menüde sadece Release olacaktır, Ok diyip çıkılır.
*- Proje ismine sag tıklayıp Build Project denir. 3 yeni warning ile Hatasız derlemesi gerekecektir.

 */
