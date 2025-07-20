/*
 * eusart.h
 *
 *  Created on: Oct 2, 2023
 *      Author: karataym
 */

#ifndef ELIB_EUSART_H_
#define ELIB_EUSART_H_

#include "stm32f3xx.h"
#include "stdio.h"
#include <stdlib.h>
#include <stdbool.h>
#include "eprogs.h"

#define UBUFFER_SIZE 254

extern uint8_t seri_data;
extern uint8_t seri_data_array[UBUFFER_SIZE];
extern uint8_t seri_index;
extern uint8_t seri_flag;

extern uint8_t blue_data;
extern uint8_t blue_data_array[UBUFFER_SIZE];
extern uint8_t blue_index;
extern uint8_t blue_flag;

extern char *usart_dbid;
extern char *usart_func;
extern char *usart_data;

extern char currentDateTimeStr[40];
extern char currentDateTimeShortStr[40];

extern uint8_t PostData[UBUFFER_SIZE];
extern uint8_t modSeriData[UBUFFER_SIZE];
extern uint16_t serial_counter;
extern uint16_t bluetooth_counter;
extern uint8_t bluetooth_connected;

extern uint8_t check_usart_process;

HAL_StatusTypeDef eSend_Transmit(UART_HandleTypeDef *huartx);
void serial_reset();
void bluetooth_reset() ;
void usart_uart_reset();
extern uint8_t usart_eid;
extern uint32_t usart_baudRate;   // Baud hızı (ör. 9600, 19200, 38400)
extern char usart_parity;        // Parity ('N' = None, 'E' = Even, 'O' = Odd)
extern uint8_t usart_dataBits;        // Veri bitleri (genelde 8)
extern uint8_t usart_stopBits;        // Stop bitleri (1 veya 2)
extern uint8_t usart_slaveID;         // Slave ID (Cihaz adresi, 1-247)
extern uint16_t usart_timeout; // Timeout süresi (ms cinsinden, ör. 1000 ms)
extern uint16_t usart_retryCount;     // Yeniden deneme sayısı (ör. 3)
extern uint16_t usart_portNumber;   // TCP için port numarası (ör. 502)
extern char usart_ipAddress[16];    // Modbus TCP için IP adresi (ör. "192.168.0.1")




/*
 * __IO uint32_t CR1;   !< USART Control register 1,                 Address offset: 0x00
 __IO uint32_t CR2;    !< USART Control register 2,                 Address offset: 0x04
 __IO uint32_t CR3;    !< USART Control register 3,                 Address offset: 0x08
 __IO uint32_t BRR;    !< USART Baud rate register,                 Address offset: 0x0C
 __IO uint32_t GTPR;   !< USART Guard time and prescaler register,  Address offset: 0x10
 __IO uint32_t RTOR;   !< USART Receiver Time Out register,         Address offset: 0x14
 __IO uint32_t RQR;    !< USART Request register,                   Address offset: 0x18
 __IO uint32_t ISR;    !< USART Interrupt and status register,      Address offset: 0x1C
 __IO uint32_t ICR;    !< USART Interrupt flag Clear register,      Address offset: 0x20
 __IO uint16_t RDR;    !< USART Receive Data register,              Address offset: 0x24
 uint16_t  RESERVED1;  !< Reserved, 0x26
 __IO uint16_t TDR;    !< USART Transmit Data register,             Address offset: 0x28
 uint16_t  RESERVED2;  !< Reserved, 0x2A
 *
 *
 #define UART_FLAG_REACK   USART_ISR_REACK         *!< UART receive enable acknowledge flag      *
 #define UART_FLAG_TEACK   USART_ISR_TEACK         *!< UART transmit enable acknowledge flag     *
 #define UART_FLAG_WUF     USART_ISR_WUF           *!< UART wake-up from stop mode flag          *
 #define UART_FLAG_RWU     USART_ISR_RWU           *!< UART receiver wake-up from mute mode flag *
 #define UART_FLAG_SBKF    USART_ISR_SBKF          *!< UART send break flag                      *
 #define UART_FLAG_CMF     USART_ISR_CMF           *!< UART character match flag                 *
 #define UART_FLAG_BUSY    USART_ISR_BUSY          *!< UART busy flag                            *
 #define UART_FLAG_ABRF    USART_ISR_ABRF          *!< UART auto Baud rate flag                  *
 #define UART_FLAG_ABRE    USART_ISR_ABRE          *!< UART auto Baud rate error                 *
 #define UART_FLAG_RTOF    USART_ISR_RTOF          *!< UART receiver timeout flag                *
 #define UART_FLAG_CTS     USART_ISR_CTS           *!< UART clear to send flag                   *
 #define UART_FLAG_CTSIF   USART_ISR_CTSIF         *!< UART clear to send interrupt flag         *
 #define UART_FLAG_LBDF    USART_ISR_LBDF          *!< UART LIN break detection flag             *
 #define UART_FLAG_TXE     USART_ISR_TXE           *!< UART transmit data register empty         *
 #define UART_FLAG_TC      USART_ISR_TC            *!< UART transmission complete                *
 #define UART_FLAG_RXNE    USART_ISR_RXNE          *!< UART read data register not empty         *
 #define UART_FLAG_IDLE    USART_ISR_IDLE          *!< UART idle flag                            *
 #define UART_FLAG_ORE     USART_ISR_ORE           *!< UART overrun error                        *
 #define UART_FLAG_NE      USART_ISR_NE            *!< UART noise error                          *
 #define UART_FLAG_FE      USART_ISR_FE            *!< UART frame error                          *
 #define UART_FLAG_PE      USART_ISR_PE            *!< UART parity error                         *
 */


#endif /* ELIB_EUSART_H_ */
