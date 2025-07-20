/*
 * edht.c
 *
 *  Created on: Mart 27, 2025
 *      Author: karataym
 */
 

#include "eprogs.h"
#include "edht_11.h"
#include <string.h> // memset için

// Global (static) değişkenler - Sadece bu dosyadan erişilebilir
static TIM_HandleTypeDef* dht_timer = NULL;
static GPIO_TypeDef* dht_port = NULL;
static uint16_t dht_pin = 0;
static uint8_t dht_initialized = 0; // Başlatılıp başlatılmadığını kontrol etmek için
DHT11_Data_t *edht_11_data;
uint16_t dht11_counter;
uint8_t dht11_read_flag = 0;

// --- Özel Fonksiyonlar ---

/**
 * @brief Belirtilen süre kadar mikrosaniye cinsinden bekler.
 * @param us: Beklenecek süre (mikrosaniye).
 * @retval Yok
 * @note Timer'ın 1MHz'de çalıştığını varsayar.
 */
static inline void DHT11_Delay_us(uint16_t us) {
    if (!dht_timer) return;
    __HAL_TIM_SET_COUNTER(dht_timer, 0);
    while (__HAL_TIM_GET_COUNTER(dht_timer) < us);
}

/**
 * @brief GPIO pinini Çıkış (Push-Pull) moduna ayarlar.
 * @retval Yok
 */
static void Set_Pin_Output(void) {
    if (!dht_port) return;
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = dht_pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP; // Push-Pull çıkış
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH; // Yüksek hız, hızlı değişimler için
    GPIO_InitStruct.Pull = GPIO_NOPULL;          // Çıkış modunda pull-up/down gereksiz
    HAL_GPIO_Init(dht_port, &GPIO_InitStruct);
}

/**
 * @brief GPIO pinini Giriş moduna ayarlar (dahili pull-up ile).
 * @retval Yok
 * @note Harici pull-up direnç kullanılıyorsa daha güvenilirdir.
 */
static void Set_Pin_Input(void) {
    if (!dht_port) return;
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = dht_pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    // ÖNEMLİ: Harici pull-up yoksa bunu kullanın. Varsa NOPULL daha iyi olabilir.
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH; // Giriş için de yüksek hız seçilebilir
    HAL_GPIO_Init(dht_port, &GPIO_InitStruct);
}

/**
 * @brief Belirli bir pin durumunun (HIGH veya LOW) ne kadar sürdüğünü ölçer.
 *        Durum değiştiğinde veya zaman aşımına uğradığında çıkar.
 * @param state: Ölçülecek pin durumu (GPIO_PIN_SET veya GPIO_PIN_RESET).
 * @param timeout_us: Maksimum bekleme süresi (mikrosaniye).
 * @param duration: Ölçülen süre (mikrosaniye) buraya yazılır.
 * @retval HAL_StatusTypeDef:
 *         - HAL_OK: Pin durumu zaman aşımından önce değişti.
 *         - HAL_TIMEOUT: Zaman aşımı oldu, pin durumu değişmedi.
 */
static HAL_StatusTypeDef Measure_Pin_State_Duration(GPIO_PinState state, uint16_t timeout_us, uint16_t *duration) {
    if (!dht_timer || !dht_port) return HAL_ERROR;

    __HAL_TIM_SET_COUNTER(dht_timer, 0);
    while (HAL_GPIO_ReadPin(dht_port, dht_pin) == state) {
        *duration = __HAL_TIM_GET_COUNTER(dht_timer);
        if (*duration > timeout_us) {
            return HAL_TIMEOUT; // Zaman aşımı
        }
    }
    // Süre zaten döngü içinde güncellendi, son değeri bir kez daha oku
    *duration = __HAL_TIM_GET_COUNTER(dht_timer);
    return HAL_OK; // Pin durumu değişti
}


// --- Genel Fonksiyonlar ---

/**
 * @brief DHT11 sürücüsünü başlatır.
 */
void DHT11_Init(TIM_HandleTypeDef *timer_handle, GPIO_TypeDef *gpio_port, uint16_t gpio_pin) {
    // Timer handle ve GPIO bilgilerini kaydet
    dht_timer = timer_handle;
    dht_port = gpio_port;
    dht_pin = gpio_pin;
    dht_initialized = 1; // Sürücü başlatıldı olarak işaretle

    // Başlangıçta pini yüksek empedanslı giriş modunda bırakmak iyi bir pratik olabilir
    Set_Pin_Input();
    HAL_Delay(100); // Sürücünün stabil hale gelmesi için küçük bir bekleme
}

/**
 * @brief DHT11 sensöründen sıcaklık ve nem verilerini okur.
 */
HAL_StatusTypeDef DHT11_Read(void) {
    uint8_t bits[5] = {0}; // 5 byte veri (Nem_T, Nem_O, Sıcaklık_T, Sıcaklık_O, Checksum)
    uint16_t duration = 0;
    dht11_read_flag = 0;
    HAL_StatusTypeDef ret = HAL_ERROR; // Başlangıçta hata durumu

    // Parametreleri ve başlatma durumunu kontrol et
    if (!dht_initialized || !edht_11_data || !dht_timer || !dht_port) {
        if (edht_11_data) edht_11_data->Status = HAL_ERROR;
        return HAL_ERROR; // Başlatılmamış veya geçersiz parametre
    }

    // Okunan verileri sıfırla
    memset(edht_11_data, 0, sizeof(DHT11_Data_t));
    edht_11_data->Status = HAL_ERROR; // Varsayılan durum

    // --- Kritik Zamanlama Bölümü Başlangıcı ---
    // Kesmeleri devre dışı bırakarak zamanlamanın bozulmasını engelle
    __disable_irq();

    // 1. Başlangıç Sinyali Gönderme
    Set_Pin_Output();                     // Pini çıkış yap
    HAL_GPIO_WritePin(dht_port, dht_pin, GPIO_PIN_RESET); // Pini LOW'a çek
    DHT11_Delay_us(18000);                // En az 18ms bekle
    HAL_GPIO_WritePin(dht_port, dht_pin, GPIO_PIN_SET);  // Pini HIGH'a çek (veya Input'a alıp pull-up'a bırak)
    DHT11_Delay_us(30);                   // 20-40us bekle
    Set_Pin_Input();                      // Pini giriş yap (Pull-up aktif olmalı)

    // 2. Sensör Yanıtını Kontrol Etme
    //    DHT11 yaklaşık 80us LOW, sonra 80us HIGH yanıtı vermeli

    // ~80us LOW yanıtını bekle ve süresini ölç (öncesindeki HIGH süresi önemli değil)
    ret = Measure_Pin_State_Duration(GPIO_PIN_SET, DHT11_TIMEOUT_US, &duration);
    if (ret != HAL_OK) goto end_read; // HIGH'dan LOW'a geçmezse timeout

    // ~80us LOW yanıtının süresini ölç
    ret = Measure_Pin_State_Duration(GPIO_PIN_RESET, DHT11_TIMEOUT_US, &duration);
    if (ret != HAL_OK) goto end_read; // LOW'da kalırsa veya çok uzun sürerse timeout
    // İsteğe bağlı: if (duration < 50 || duration > 110) ret = HAL_TIMEOUT;

    // ~80us HIGH yanıtının süresini ölç
    ret = Measure_Pin_State_Duration(GPIO_PIN_SET, DHT11_TIMEOUT_US, &duration);
    if (ret != HAL_OK) goto end_read; // HIGH'da kalırsa veya çok uzun sürerse timeout
    // İsteğe bağlı: if (duration < 50 || duration > 110) ret = HAL_TIMEOUT;


    // 3. Veri Bitlerini Okuma (40 bit = 5 byte)
    for (int i = 0; i < 5; i++) { // Her byte için
        for (int j = 7; j >= 0; j--) { // Her bit için (MSB first)
            // Her bitin başındaki ~50us LOW süresini ölç/atla
            ret = Measure_Pin_State_Duration(GPIO_PIN_RESET, DHT11_TIMEOUT_US, &duration);
            if (ret != HAL_OK) goto end_read;
            // İsteğe bağlı: if (duration < 30 || duration > 70) ret = HAL_TIMEOUT;

            // Bit değerini belirleyen HIGH süresini ölç
            ret = Measure_Pin_State_Duration(GPIO_PIN_SET, DHT11_TIMEOUT_US, &duration);
            if (ret != HAL_OK) goto end_read;

            // Süre eşik değerinden büyükse biti '1' olarak kaydet
            if (duration > DHT11_BIT_THRESHOLD_US) {
                bits[i] |= (1 << j);
            }
        }
    }

    // --- Kritik Zamanlama Bölümü Sonu ---
end_read:
    __enable_irq(); // Kesmeleri tekrar etkinleştir

    // 4. Checksum Kontrolü ve Veri İşleme
    if (ret == HAL_OK) {
        uint8_t calculated_checksum = bits[0] + bits[1] + bits[2] + bits[3];
        if (calculated_checksum == bits[4]) {
            // Checksum doğru, verileri yapıya kopyala
            edht_11_data->Hum_Int = bits[0];
            edht_11_data->Hum_Dec = bits[1]; // DHT11 için genellikle 0
            edht_11_data->Temp_Int = bits[2];
            edht_11_data->Temp_Dec = bits[3]; // DHT11 için genellikle 0
            edht_11_data->Checksum = bits[4];

            // Float değerleri hesapla (DHT11 için ondalık kısımlar genelde 0'dır)
            edht_11_data->Humidity = (float)edht_11_data->Hum_Int + ((float)edht_11_data->Hum_Dec / 10.0f);
            edht_11_data->Temperature = (float)edht_11_data->Temp_Int + ((float)edht_11_data->Temp_Dec / 10.0f);

            // İşaret biti kontrolü (DHT11'de genellikle olmaz ama DHT22 için gerekebilir)
            // if (edht_11_data->Temp_Int & 0x80) { edht_11_data->Temperature *= -1; }
            edht_11_data->Status = HAL_OK; // Başarılı
            return HAL_OK;
        } else {
            // Checksum hatası
            edht_11_data->Status = HAL_ERROR;
            return HAL_ERROR;
        }
    } else {
        // Zaman aşımı oldu
        edht_11_data->Status = HAL_TIMEOUT;
        return HAL_TIMEOUT;
    }
}
/*
Timer Yapılandırması: Bu kod, TIM_HandleTypeDef ile temsil edilen bir timer'ın önceden 1 MHz frekansında (yani 1 tik = 1 mikrosaniye) sayacak şekilde yapılandırıldığını ve HAL_TIM_Base_Start() ile başlatıldığını varsayar. Bunu STM32CubeMX kullanarak kolayca yapabilirsiniz:
Bir timer seçin (örn. TIM2, TIM3...).
Clock Source'u "Internal Clock" olarak ayarlayın.
Prescaler değerini (TimerClockFreq / 1000000) - 1 olarak hesaplayın. Örneğin, timer'ınız 72MHz'lik bir bus'a bağlıysa, Prescaler 71 olur ((72000000 / 1000000) - 1).
Counter Period (ARR) değerini maksimumda bırakabilirsiniz (örn. 65535 for 16-bit timer).
Harici Pull-Up: DHT11 veri pinine (PC3) harici bir 4.7kΩ veya 10kΩ pull-up direnci bağlamanız şiddetle tavsiye edilir. Kodda dahili pull-up etkinleştirilse de, harici direnç genellikle daha güvenilir sonuç verir.
if (DHT11_Read(&dht_data) == HAL_OK) {
     // Okuma başarılı
     printf("Sıcaklık: %.1f C, Nem: %.1f %%\r\n", dht_data.Temperature, dht_data.Humidity);

     // LCD'ye yazdırma örneği
     // sprintf(lcd_buffer, "T:%.1fC H:%.1f%%", dht_data.Temperature, dht_data.Humidity);
     // LCD_Send_String(lcd_buffer);

  } else {
     // Okuma başarısız (Timeout veya Checksum hatası)
     printf("DHT11 Okuma Hatası! Status: %d\r\n", dht_data.Status);
     // Hata durumunu ele alın
  }

  HAL_Delay(2000); // Sensörü çok sık okumayın (DHT11 için min 1-2 saniye)
*/
