#ifndef INC_DHT11_H_
#define INC_DHT11_H_

#include "stm32f3xx.h"
//#include "stm32f3xx_hal.h" // Kullandığınız MCU ailesine göre değiştirin (örn. stm32f1xx_hal.h)
#include <stdint.h>

// --- Ayarlar ---
// Timeout süresi (mikrosaniye). Sinyal değişimleri için maksimum bekleme süresi.
#define DHT11_TIMEOUT_US         120
// '0' ve '1' bitlerini ayırt etmek için kullanılan HIGH pals süresi eşiği (mikrosaniye).
// Tipik '0': ~28us, Tipik '1': ~70us. 40-50us iyi bir eşik olabilir.
#define DHT11_BIT_THRESHOLD_US   45
// --- Ayarlar Sonu ---

// Okunan verileri ve durumu tutacak yapı
typedef struct {
    float Temperature;    // Sıcaklık (°C)
    float Humidity;       // Nem (%)
    uint8_t Temp_Int;     // Sıcaklık Tamsayı Kısmı
    uint8_t Temp_Dec;     // Sıcaklık Ondalık Kısmı (DHT11 için genellikle 0)
    uint8_t Hum_Int;      // Nem Tamsayı Kısmı
    uint8_t Hum_Dec;      // Nem Ondalık Kısmı (DHT11 için genellikle 0)
    uint8_t Checksum;     // Okunan Checksum
    HAL_StatusTypeDef Status; // Son okumanın durumu (HAL_OK, HAL_TIMEOUT, HAL_ERROR)
} DHT11_Data_t;
extern DHT11_Data_t *edht_11_data;
extern uint16_t dht11_counter;


/**
 * @brief DHT11 sürücüsünü başlatır.
 * @param timer_handle: Mikrosaniye sayımı için yapılandırılmış ve başlatılmış Timer Handle.
 *                      (1MHz frekansında çalışmalıdır - 1 tick = 1us)
 * @param gpio_port: DHT11 veri pininin bağlı olduğu GPIO portu (örn. GPIOC).
 * @param gpio_pin: DHT11 veri pininin numarası (örn. GPIO_PIN_3).
 * @retval Yok
 * @note Bu fonksiyon çağrılmadan önce timer başlatılmış olmalıdır (HAL_TIM_Base_Start).
 */
void DHT11_Init(TIM_HandleTypeDef *timer_handle, GPIO_TypeDef *gpio_port, uint16_t gpio_pin);

/**
 * @brief DHT11 sensöründen sıcaklık ve nem verilerini okur.
 * @param data: Okunan verilerin ve durumun yazılacağı DHT11_Data_t yapısının adresi.
 * @retval HAL_StatusTypeDef:
 *         - HAL_OK: Okuma başarılı ve checksum doğru.
 *         - HAL_TIMEOUT: Sensör yanıt vermedi veya okuma sırasında zaman aşımı oldu.
 *         - HAL_ERROR: Checksum hatası.
 *         - HAL_ERROR (diğer): Sürücü başlatılmamış veya geçersiz parametre.
 * @note Okuma işlemi sırasında (~5ms) kesmeler kısa süreliğine devre dışı bırakılır.
 */
HAL_StatusTypeDef DHT11_Read(void);


#endif /* INC_DHT11_H_ */
