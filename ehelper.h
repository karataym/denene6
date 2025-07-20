#ifndef __LIB_OF_HELPER_FUNCTIONS_H
#define __LIB_OF_HELPER_FUNCTIONS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#ifdef __has_include
  #if __has_include("netif.h")
	#include "lwip/netif.h" // lwIP netif yapısı
  #endif
#endif


// Debug UART tanımı
extern UART_HandleTypeDef *debug_huart;
#define MAC_ADDRESS_LENGTH 6
#define MAC_ADDRESS_STRING_LENGTH 18
extern uint8_t custom_mac_address[MAC_ADDRESS_LENGTH];

/**
 * @brief Helper kütüphanesini başlatır
 * @param project_name Proje ismi
 * @param huart Debug çıktıları için kullanılacak UART
 * @note printf çıktıları bu UART'a yönlendirilir
 */
void helper_init(const char *project_name, UART_HandleTypeDef *huart);

/**
 * @brief printf için UART write fonksiyonu
 * @param ptr Yazılacak veri
 * @param len Veri uzunluğu
 * @return Yazılan byte sayısı
 */
int uart_write(char *ptr, int len);
/**
 * @brief printf için Mikroislemciden mac_address fonksiyonu
 * @param ptr Yazılacak veri
 * @param len Veri uzunluğu
 * @return Yazılan byte sayısı
 */
void generate_unique_mac(void);
/**
 * @brief printf için Mikroislemciden barcode fonksiyonu
 * @param ptr Yazılacak veri
 * @param len Veri uzunluğu
 * @return Yazılan byte sayısı
 */
void generate_unique_barcode(char *barcode_str, uint16_t company_id, char *project_id, char *function_id, char *erelase);
/**
 * @brief printf için Mikroislemciden barcode fonksiyonu
 * @param ptr Yazılacak veri
 * @param len Veri uzunluğu
 * @return Yazılan byte sayısı
 */
void reform_hardware_code(char *barcode_str, uint16_t hw_companyid, char *hw_projectid, char *hw_functionid, char *hw_erelase);
/**
 * @brief printf için Mikroislemciden barcode fonksiyonu
 * @param ptr Yazılacak veri
 * @param len Veri uzunluğu
 * @return Yazılan byte sayısı
 */
void reform_software_code(char *barcode_str, uint16_t sw_companyid, char *sw_projectid, char *sw_functionid, char *sw_erelase);

/**
 * @brief verilen string fonksionundan bir indexinden veri döndürür 
 * @param index kaçıncı sıradaki değeri
 * @param delimiter stringin ayraç karekteri
 * @return dönen string
 */
char* str_parser(char *xstring, char const *delimiter, int index);

float Celsius2Fahrenheit(float invalue);
float Fahrenheit2Celsius(float invalue);
uint8_t FloatToByte(float decimal_part);
float ByteToFloat(uint8_t f_byte, uint8_t d_byte);
void estrcpy(char* dest, const char* src, size_t dest_size);
uint8_t string_to_mac(const char *mac_str, uint8_t *mac);
void mac_to_string(const uint8_t mac[6], char *str);
uint8_t is_local_mac(const char *mac);
uint8_t is_valid_mac(const char *mac);
void print_mac(char *mac_str);


#ifdef __cplusplus
}
#endif

#endif /* __LIB_OF_HELPER_FUNCTIONS_H */ 
