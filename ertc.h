/*
 * ertc.h
 *
 *  Created on: Oct 25, 2024
 *      Author: mehme
 */

#ifndef ELIB_ERTC_H_
#define ELIB_ERTC_H_

#include "stm32f3xx.h"
#include <stdlib.h>
#include "stm32f3xx_hal_rtc.h"

#include "main.h"
#include <string.h>
#include <stdio.h>
#include "stdbool.h"
#include <time.h> // mktime ve difftime için gerekli
#include <math.h>   // fabs için gerekli (difftime sonucu double olduğu için)


typedef struct {
    uint8_t seconds;
    uint8_t minutes;
    uint8_t hours;
    uint8_t day;
    uint8_t month;
    uint16_t year;
} ChangeDateTimeTypeDef;
typedef struct {
    uint32_t prevHours;
    uint32_t prevMinutes;
    uint32_t prevSeconds;
} eTimeStruct;
// Structure to hold time components

#define uyear 2000

typedef struct {
//    uint8_t seconds;
//    uint32_t SubSeconds;
//    uint8_t minutes;
//    uint8_t hours;
//    uint16_t day;
//    uint8_t month;
//    uint16_t year;
//    uint8_t weekday;
    // Previous values for change detection
    uint8_t previousSeconds;
    uint8_t previousMinutes;
    uint8_t previousHours;
    uint16_t previousDay;
	uint8_t previousMonth;
	uint16_t previousYear;
	eTimeStruct currenteTimeStruct;
	eTimeStruct prevTime;
    RTC_TimeTypeDef *rtcTime;  // Raw RTC time
    RTC_DateTypeDef *rtcDate;  // Raw RTC date
} TimeComponents;
// Data structure for extended time information (if needed)
typedef struct {
    char utc_string[50];
    double utc_float_timestamp;
    char utc_timezone[10];
    char current_string[50];
    double current_float_timestamp;
    char current_timezone[10];
    char source[10];
    char source_address[30];
    char input_currenttime[80];
} ExtendedTimeData;

// External variables
//extern RTC_TimeTypeDef rtcTime;
//extern RTC_DateTypeDef rtcDate;
extern RTC_HandleTypeDef hrtc;
extern TimeComponents delayTimes[4];
extern TimeComponents currentTime, rtcTimeComponents, timeDifference, lastAlertTime;

extern char currentDateTimeStr[40];
extern char currentDateTimeShortStr[40];

// Function Prototypes
HAL_StatusTypeDef rtc_init(void);
HAL_StatusTypeDef rtc_enable_seconds_interrupt(void);
void RTC_IRQHandler(void);
void HAL_RTC_AlarmAEventCallback(RTC_HandleTypeDef *hrtc);
void HAL_RTCEx_RTCSEC_IRQHandler(RTC_HandleTypeDef *hrtc);

void resetDateTime(void);
HAL_StatusTypeDef setRtcDateTime(uint16_t year, uint8_t month, uint8_t day, uint8_t weekday,
                               uint8_t hours, uint8_t minutes, uint8_t seconds);
HAL_StatusTypeDef updateCurrentDateTimeFromRtc(void);
HAL_StatusTypeDef updateRtcFromTimeString(const char *timeString);
uint8_t hasSecondChanged(void);
uint8_t hasMinuteChanged(void);
uint8_t hasHourChanged(void);
void copyTimeComponents(TimeComponents *dest, const TimeComponents *src);
long calculateTotalSeconds(const TimeComponents *time);
void convertSecondsToTimeComponents(long totalSeconds, TimeComponents *time);
long calculateTimeDifference(const TimeComponents *time1, const TimeComponents *time2);
long getTimeDifferenceSeconds(void);
long getDelayPassedSeconds(uint8_t index);
long getTimeDifferenceMinutes(const TimeComponents *time1, const TimeComponents *time2);
long getTimeDifferenceHours(const TimeComponents *time1, const TimeComponents *time2);
long getTimeDifferenceDays(const TimeComponents *time1, const TimeComponents *time2);

uint8_t isSecondMultipleOf(uint32_t seconds);
uint8_t isMinuteMultipleOf(uint32_t minutes);
bool hasMinutesPassed(uint32_t intervalMinutes);
bool hasSecondsPassed(uint32_t intervalSeconds);

bool hasSecondsChanged(void);
bool hasMinutesChanged(void);
bool hasHoursChanged(void);
bool hasDayChanged(void);
bool hasMonthChanged(void);
bool hasYearChanged(void);
HAL_StatusTypeDef rtc_enable_wakeup_interrupt_1sec(void);
/*
 * İsimlendirme: Değişkenler ve fonksiyonlar için daha açıklayıcı isimler kullanıldı (örneğin, eclock yerine currentTime, ecall_datetime yerine updateCurrentDateTimeFromRtc).
Veri Yapıları:
SetClockType yerine TimeComponents kullanıldı ve önceki zaman değerlerini (ee_saniye, ee_dakika, ee_saat) bu yapıya taşındı. Bu, zaman değişikliklerini algılamak için daha organize bir yaklaşım sağlıyor.
ExtendedTimeData yapısı, zamanla ilgili ek bilgileri tutmak için tanımlandı (örneğin, zaman dizeleri, zaman damgaları).
Fonksiyonel İyileştirmeler:
resetDateTime(): Zamanı sıfırlamak için kullanılıyor.
setRtcDateTime(): RTC'ye tarih ve zamanı ayarlamak için kullanılıyor ve hata yönetimi için HAL_StatusTypeDef döndürüyor.
updateCurrentDateTimeFromRtc(): RTC'den tarih ve zamanı alıp currentTime yapısını güncelliyor ve hata yönetimi için HAL_StatusTypeDef döndürüyor.
updateRtcFromTimeString(): Zaman dizesini ayrıştırıp RTC'yi güncelliyor ve hata yönetimi için HAL_StatusTypeDef döndürüyor. strncpy yerine sscanf kullanıldı ve sscanf dönüş değeri kontrol ediliyor.
Zaman farkı hesaplama fonksiyonları (calculateTimeDifference, getTimeDifferenceSeconds, vb.) daha genel hale getirildi ve isimleri kısaltıldı.
getRtcYear(), getRtcMonth(), vb. fonksiyonlar, updateCurrentDateTimeFromRtc() fonksiyonunun bir kez çağrılması ve currentTime yapısından değerlerin alınması şeklinde optimize edildi.
isSecondMultipleOf(): Saniyenin belirli bir değere bölünüp bölünmediğini kontrol etmek için kullanılıyor (eski check_minute).
RTC Kesme Desteği:
rtc_init(): RTC'yi başlatmak için (örneğin, saat kaynağını seçmek).
rtc_enable_seconds_interrupt(): RTC saniye kesmesini etkinleştirmek için.
RTC_IRQHandler(): Genel kesme işleyici.
HAL_RTC_AlarmAEventCallback(): Alarm A kesmesi için geri çağrı fonksiyonu (kullanılacaksa).
HAL_RTCEx_RTCSEC_IRQHandler(): Saniye kesmesi için geri çağrı fonksiyonu. Bu fonksiyon içinde, her saniyede yapılacak işlemler gerçekleştirilebilir (örneğin, bir LED'i yakıp söndürmek).
Hata Yönetimi:
HAL_StatusTypeDef dönüş değerleri kullanılarak hata yönetimi iyileştirildi.
sscanf dönüş değeri kontrol ediliyor.
Kodun Belgelenmesi:
Kodun daha iyi anlaşılması için yorumlar eklendi.
Nasıl Kullanılır?

RTC Başlatma: Uygulamanızın başlangıcında rtc_init() fonksiyonunu çağırarak RTC'yi başlatın.
Saniye Kesmesini Etkinleştirme (Opsiyonel): Her saniyede işlem yapmak istiyorsanız, rtc_enable_seconds_interrupt() fonksiyonunu çağırarak saniye kesmesini etkinleştirin.
Kesme İşleyici: STM32'nin kesme işleyicisinde RTC_IRQHandler() fonksiyonunu çağırın.
Saniye Kesme Geri Çağrısı: HAL_RTCEx_RTCSEC_IRQHandler() fonksiyonunu uygulamanızda geçersiz kılın (override) ve her saniyede yapmak istediğiniz işlemleri bu fonksiyonun içine yazın.
Zamanı Ayarlama: setRtcDateTime() veya updateRtcFromTimeString() fonksiyonlarını kullanarak RTC'yi ayarlayın.
Zamanı Almak: updateCurrentDateTimeFromRtc() fonksiyonunu kullanarak güncel zamanı alın ve getRtcYear(), getRtcMonth(), vb. fonksiyonları kullanarak zaman bileşenlerine erişin.
Zaman Farkı Hesaplama: calculateTimeDifference() veya getTimeDifferenceSeconds() gibi fonksiyonları kullanarak zaman farklarını hesaplayın.
Periyodik İşlemler: isSecondMultipleOf() fonksiyonunu kullanarak belirli saniye aralıklarında işlem yapın.


#include "main.h"
#include "ertc.h"

RTC_HandleTypeDef hrtc; // RTC handle (global veya extern olarak tanımlanmalı)

void SystemClock_Config(void); // Sistem saatini yapılandıran fonksiyon (CubeMX veya benzeri araçlarla oluşturulur)

int main(void) {
    HAL_Init();
    SystemClock_Config();

    // RTC'yi başlat
    if (rtc_init() != HAL_OK) {
        Error_Handler(); // Hata durumunu işle
    }

    // RTC'den ilk okuma (gerekirse)
    updateCurrentDateTimeFromRtc();

    // Saniye kesmesini etkinleştir (eğer her saniye işlem yapacaksanız)
    if (rtc_enable_seconds_interrupt() != HAL_OK) {
        Error_Handler(); // Hata durumunu işle
    }

    while (1) {
        // Diğer uygulama kodları buraya
        // Örneğin, zamanı belirli aralıklarla kontrol etmek:
        if (isSecondMultipleOf(5)) { // Her 5 saniyede bir
            printf("5 saniye geçti! Zaman: %02d:%02d:%02d\n", getRtcHour(), getRtcMinute(), getRtcSecond());
        }
        HAL_Delay(100); // Küçük bir gecikme (gerekirse)
    }
}

void Error_Handler(void) {
    // Hata işleme kodları buraya
    while (1) {
        // Örneğin, bir LED'i sürekli yakıp söndürebilirsiniz.
    }
}

// Sistem saatini yapılandıran fonksiyon (CubeMX veya benzeri araçlarla oluşturulur)
void SystemClock_Config(void) {
    // ... (Saat yapılandırma kodları)
}

// HAL_RTC_MspInit fonksiyonu (RTC'yi başlatmak için gerekli donanım yapılandırması)
void HAL_RTC_MspInit(RTC_HandleTypeDef* hrtc) {
    if(hrtc->Instance == RTC) {
        __HAL_RCC_BKP_CLK_ENABLE(); // Backup domain clock'u etkinleştir
        __HAL_RCC_RTC_ENABLE();     // RTC clock'u etkinleştir

        // Kesme önceliğini ayarla (NVIC)
        HAL_NVIC_SetPriority(RTC_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(RTC_IRQn);
    }
}
stm32f3xx_it.c

#include "stm32f3xx_it.h"
#include "ertc.h"
#include "main.h" // main.c'deki tanımlara erişmek için (örneğin hrtc)

extern RTC_HandleTypeDef hrtc; // main.c'de tanımlanan hrtc'yi burada kullanabilmek için extern olarak tanımlayın

void RTC_IRQHandler(void) {
    HAL_RTC_IRQHandler(&hrtc);
}

void HAL_RTCEx_RTCSEC_IRQHandler(RTC_HandleTypeDef *hrtc) {
    // Bu fonksiyon her saniyede bir çağrılır
    // Örneğin, bir LED'i yakıp söndürebilirsiniz:
    HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_0);

    // Gerekirse zamanı güncelleyebilirsiniz:
    updateCurrentDateTimeFromRtc();

    // Diğer saniye bazlı işlemler burada yapılabilir
    printf("Saniye kesmesi! Zaman: %02d:%02d:%02d\n", getRtcHour(), getRtcMinute(), getRtcSecond());
}

// Eğer Alarm A kesmesini de kullanıyorsanız, bu fonksiyonu da tanımlayabilirsiniz:
void HAL_RTC_AlarmAEventCallback(RTC_HandleTypeDef *hrtc) {
    // Alarm A kesmesi gerçekleştiğinde yapılacak işlemler buraya
}

main içine
* Private user code ---------------------------------------------------------*
* USER CODE BEGIN 0 *
HAL_StatusTypeDef rtc_enable_wakeup_interrupt_1sec(void) {
    // RTC Wakeup Timer'ı saniyede bir kesme üretecek şekilde ayarla
    // RTC Saat Kaynağı: LSE (32768 Hz)
    // Wakeup Clock Bölücüsü: RTC_WAKEUPCLOCK_RTCCLK_DIV16 (32768 / 16 = 2048 Hz)
    // İstenen Kesme Sıklığı: 1 Hz (saniyede bir)
    // Gerekli Sayaç Değeri: 2048 - 1 = 2047

    // HAL_RTCEx_SetWakeUpTimer_IT fonksiyonunu kullanmak daha doğrudur,
    // çünkü bu fonksiyon aynı zamanda kesmeyi de etkinleştirir.
    if (HAL_RTCEx_SetWakeUpTimer_IT(&hrtc, 2047, RTC_WAKEUPCLOCK_RTCCLK_DIV16) != HAL_OK) {
        return HAL_ERROR; // Hata oluştu
    }

    // NVIC (Nested Vectored Interrupt Controller) ayarları
    // RTC Wakeup kesmesi RTC_WKUP_IRQn vektörünü kullanır.
    HAL_NVIC_SetPriority(RTC_WKUP_IRQn, 0, 0); // Öncelik (0 en yüksek)
    HAL_NVIC_EnableIRQ(RTC_WKUP_IRQn);

    return HAL_OK;
}

// int main(void) içine
* USER CODE END 0 *
* USER CODE BEGIN 2 *
  if (rtc_enable_wakeup_interrupt_1sec() != HAL_OK)
  {
      Error_Handler();
  }

  * USER CODE END 2 *


static void MX_RTC_Init(void) içinde olmalıdır.
** Enable the WakeUp
  *
  if (HAL_RTCEx_SetWakeUpTimer(&hrtc, 0, RTC_WAKEUPCLOCK_RTCCLK_DIV16) != HAL_OK)
  {
    Error_Handler();
  }
  * USER CODE BEGIN RTC_Init 2 *

Burada en sonda olması zorunludur.
* USER CODE BEGIN 4 *
void HAL_RTCEx_WakeUpTimerEventCallback(RTC_HandleTypeDef *hrtc)
{
    // Her saniye yapılacak işlem buraya yazılır
    HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin); // Nucleo kartındaki yeşil LED (pin adı LD2_Pin olmayabilir)

    // WakeUpTimer bayrağı HAL_RTCEx_WakeUpTimerIRQHandler tarafından otomatik temizlenir.
}
* USER CODE END 4 *
*/
#endif /* ELIB1_ERTC_H_ */
