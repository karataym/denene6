/*
 * ertc.c
 *
 *  Created on: Oct 25, 2024
 *      Author: mehme
 */

#include "ertc.h"

ExtendedTimeData extendedTimeData;
TimeComponents delayTimes[4]; // Use TimeComponents instead of SetClockType
TimeComponents currentTime, rtcTimeComponents, timeDifference, lastAlertTime; // Rename eclock, e_rtc, eclock_remiander, elast_alert
//RTC_TimeTypeDef rtcTime;
//RTC_DateTypeDef rtcDate;
char currentDateTimeStr[40];
char currentDateTimeShortStr[40];

//static TimeStruct prevTime = {0, 0, 0}; // Önceki zamanı tutar
static uint8_t isFirstRun = 1; // İlk çalıştırmayı kontrol eder

// Zaman farkını saniye cinsinden hesaplayan yardımcı fonksiyon
uint32_t getTimeDifferenceInSeconds(eTimeStruct current, eTimeStruct prev) {
    // Toplam saniyeyi hesapla
    uint32_t currentTotalSeconds = current.prevHours * 3600 + current.prevMinutes * 60 + current.prevSeconds;
    uint32_t prevTotalSeconds = prev.prevHours * 3600 + prev.prevMinutes * 60 + prev.prevSeconds;
    // Saniye farkını döndür
    return currentTotalSeconds - prevTotalSeconds;
}
// Belirli bir saniye aralığı geçti mi kontrol eden fonksiyon
bool hasSecondsPassed(uint32_t intervalSeconds) {
    // İlk çalıştırmada prevTime'ı başlat
    if (isFirstRun) {
        currentTime.prevTime.prevHours = currentTime.rtcTime->Hours;
        currentTime.prevTime.prevMinutes =  currentTime.rtcTime->Minutes;
        currentTime.prevTime.prevSeconds = currentTime.rtcTime->Seconds;
        isFirstRun = 0;
        return false; // İlk çalıştırmada false döndür
    }
    // Zaman farkını hesapla
    uint32_t diffSeconds = getTimeDifferenceInSeconds(currentTime.currenteTimeStruct, currentTime.prevTime);
    // Eğer istenen saniye aralığı geçtiyse
    if (diffSeconds >= intervalSeconds) {
    	currentTime.prevTime = currentTime.currenteTimeStruct; // prevTime'ı güncelle
        return true; // Aralığı geçti
    }
    return false; // Aralığı geçmedi
}
// Belirli bir dakika aralığı geçti mi kontrol eden fonksiyon
bool hasMinutesPassed(uint32_t intervalMinutes) {
    // İlk çalıştırmada prevTime'ı başlat
    if (isFirstRun) {
    	currentTime.prevTime.prevHours = currentTime.rtcTime->Hours;
		currentTime.prevTime.prevMinutes =  currentTime.rtcTime->Minutes;
		currentTime.prevTime.prevSeconds = currentTime.rtcTime->Seconds;
        isFirstRun = 0;
        return false; // İlk çalıştırmada false döndür
    }
    // Zaman farkını saniye cinsinden hesapla
    uint32_t diffSeconds = getTimeDifferenceInSeconds(currentTime.currenteTimeStruct, currentTime.prevTime);
    uint32_t diffMinutes = diffSeconds / 60;

    // Eğer istenen dakika aralığı geçtiyse
    if (diffMinutes >= intervalMinutes && currentTime.prevTime.prevSeconds < 2) {
    	currentTime.prevTime.prevHours = currentTime.rtcTime->Hours;
		currentTime.prevTime.prevMinutes = currentTime.rtcTime->Minutes;
		currentTime.prevTime.prevSeconds = currentTime.rtcTime->Seconds; // prevTime'ı güncelle
        return true; // Aralığı geçti
    }
    return false; // Aralığı geçmedi
}
// --- Değişiklik Kontrol Fonksiyonları ---
bool hasSecondsChanged(void) { return (currentTime.rtcTime->Seconds != currentTime.previousSeconds); }
bool hasMinutesChanged(void) { return (currentTime.rtcTime->Minutes != currentTime.previousMinutes); }
bool hasHoursChanged(void)   { return (currentTime.rtcTime->Hours != currentTime.previousHours); }
bool hasDayChanged(void)     { return (currentTime.rtcDate->Date != currentTime.previousDay); }
bool hasMonthChanged(void)   { return (currentTime.rtcDate->Month != currentTime.previousMonth); }
bool hasYearChanged(void)    { return (currentTime.rtcDate->Year != currentTime.previousYear); }

// Function to reset the date and time
void resetDateTime(void) {
	currentTime.rtcTime->Seconds = 0;
	currentTime.rtcTime->Minutes = 0;
	currentTime.rtcTime->Hours = 0;
	currentTime.rtcDate->Date = 0;
	currentTime.rtcDate->Month = 0;
	currentTime.rtcDate->Year = 0; // RTC year base
    currentTime.rtcDate->WeekDay = 0;
}

// Function to set the RTC date and time
HAL_StatusTypeDef setRtcDateTime(uint16_t year, uint8_t month, uint8_t day, uint8_t weekday,
                               uint8_t hours, uint8_t minutes, uint8_t seconds) {
	currentTime.rtcTime->Hours = hours;
	currentTime.rtcTime->Minutes = minutes;
	currentTime.rtcTime->Seconds = seconds;
    if (HAL_RTC_SetTime(&hrtc, currentTime.rtcTime, RTC_FORMAT_BIN) != HAL_OK) {
        return HAL_ERROR;
    }
    currentTime.rtcDate->WeekDay = weekday;
    currentTime.rtcDate->Month = month;
    currentTime.rtcDate->Date = day;
    currentTime.rtcDate->Year = year - 2000; // Adjust year for RTC format
    if (HAL_RTC_SetDate(&hrtc, currentTime.rtcDate, RTC_FORMAT_BIN) != HAL_OK) {
        return HAL_ERROR;
    }
    return HAL_OK;
}
void update_init(void) {
	HAL_RTCEx_SetSmoothCalib(&hrtc, RTC_SMOOTHCALIB_PERIOD_32SEC, RTC_SMOOTHCALIB_PLUSPULSES_SET, 0);
	//DS1307_SetInterruptRate(DS1307_32768HZ);
}
//HAL_StatusTypeDef Check_ds1307_clock(I2C_HandleTypeDef *hi2c) {
//	//return HAL_I2C_IsDeviceReady(hi2c, (uint16_t) DS1307_I2C_CLOCK<<1, 10, 100);
//}
//HAL_StatusTypeDef Check_ds1307_eprom(I2C_HandleTypeDef *hi2c) {
//	//return HAL_I2C_IsDeviceReady(hi2c, (uint16_t)DS1307_I2C_EPROM<<1, 10, 100);
//}
// TimeComponents yapısını, C'nin standart <time.h> kütüphanesinin kullandığı
// Unix Zaman Damgasına (1970'den beri geçen saniye) çevirir.
// Bu, ay ve yıl geçişlerini doğru hesaplamanın TEK GÜVENİLİR YOLUDUR.
//static time_t Convert_To_Unix_Timestamp(const TimeComponents *t) {
//    struct tm timeinfo = {0};
//    timeinfo.tm_year = t->year - 1900; // tm_year: 1900'dan beri geçen yıl sayısı
//    timeinfo.tm_mon  = t->month - 1;   // tm_mon: 0-11 (Ocak=0)
//    timeinfo.tm_mday = t->day;
//    timeinfo.tm_hour = t->hours;
//    timeinfo.tm_min  = t->minutes;
//    timeinfo.tm_sec  = t->seconds;
//    timeinfo.tm_isdst = -1; // Gün ışığından yararlanma durumunu otomatik algıla
//    return mktime(&timeinfo);
//}
// Zamanı string formatına dönüştürür.
void fillCurrentTimeString() {
	snprintf(currentDateTimeStr, sizeof(currentDateTimeStr), "%4u-%02u-%02u %02u:%02u:%02u %u",
			currentTime.rtcDate->Year+uyear, currentTime.rtcDate->Month, currentTime.rtcDate->Date,
			currentTime.rtcTime->Hours, currentTime.rtcTime->Minutes, currentTime.rtcTime->Seconds,
			currentTime.rtcDate->WeekDay);
	snprintf(currentDateTimeShortStr, sizeof(currentDateTimeShortStr), "%4d%02d%02d_%02d%02d%02d",
			currentTime.rtcDate->Year+uyear, currentTime.rtcDate->Month, currentTime.rtcDate->Date,
			currentTime.rtcTime->Hours, currentTime.rtcTime->Minutes, currentTime.rtcTime->Seconds);
}
// Function to update the current date and time from the RTC
HAL_StatusTypeDef updateCurrentDateTimeFromRtc(void) {
    //__disable_irq(); // Kesintileri geçici devre dışı bırak

	// Önceki değerleri güncellemeden önce mevcut değerleri sakla
	currentTime.previousSeconds = currentTime.rtcTime->Seconds;
	currentTime.previousMinutes = currentTime.rtcTime->Minutes;
	currentTime.previousHours = currentTime.rtcTime->Hours;
	currentTime.previousDay = currentTime.rtcDate->Date;
	currentTime.previousMonth = currentTime.rtcDate->Month;
	currentTime.previousYear = currentTime.rtcDate->Year;

    currentTime.currenteTimeStruct.prevHours = currentTime.rtcTime->Hours;
	currentTime.currenteTimeStruct.prevMinutes = currentTime.rtcTime->Minutes;
	currentTime.currenteTimeStruct.prevSeconds = currentTime.rtcTime->Seconds;

    // Get the current time
    if (HAL_RTC_GetTime(&hrtc, currentTime.rtcTime, RTC_FORMAT_BIN) != HAL_OK) {
    	return HAL_ERROR;
    }
    // Get the current date
	if (HAL_RTC_GetDate(&hrtc, currentTime.rtcDate, RTC_FORMAT_BIN) != HAL_OK) {
		return HAL_ERROR;
	}
    //__enable_irq();


    fillCurrentTimeString();
    return HAL_OK;
}

// Function to parse a time string and update RTC
HAL_StatusTypeDef updateRtcFromTimeString(const char *timeString) {
    int year, month, day, hour, minute, second;
    if (sscanf(extendedTimeData.input_currenttime, "%d-%d-%dT %d:%d:%d", &year, &month, &day, &hour, &minute, &second) != 6) {
        return HAL_ERROR; // Handle parsing error
    }

    HAL_StatusTypeDef ret = setRtcDateTime(year, month, day, 0, hour, minute, second); // Weekday can be calculated if needed
    if (ret != HAL_OK) {
        return ret;
    }

    printf("RTC updated: %s\n", extendedTimeData.input_currenttime);
    return HAL_OK;
}

// Function to copy TimeComponents
void copyTimeComponents(TimeComponents *dest, const TimeComponents *src) {
    if (dest == NULL || src == NULL) return;
    *dest = *src;
}

// Function to calculate total seconds from TimeComponents
long calculateTotalSeconds(const TimeComponents *time) {
    return time->rtcTime->Seconds + (60 * time->rtcTime->Minutes) + (3600 * time->rtcTime->Hours) + (86400 * time->rtcDate->Date);
}

// Function to convert total seconds to TimeComponents
void convertSecondsToTimeComponents(long totalSeconds, TimeComponents *time) {
	time->rtcDate->Date = totalSeconds / 86400;
    totalSeconds %= 86400;
    time->rtcTime->Hours = totalSeconds / 3600;
    totalSeconds %= 3600;
    time->rtcTime->Minutes = totalSeconds / 60;
    time->rtcTime->Seconds = totalSeconds % 60;
}

// Generic function to calculate time difference in seconds
long calculateTimeDifference(const TimeComponents *time1, const TimeComponents *time2) {
    if (time1 == NULL || time2 == NULL) return 0;
    long totalSeconds1 = calculateTotalSeconds(time1);
    long totalSeconds2 = calculateTotalSeconds(time2);
    return labs(totalSeconds1 - totalSeconds2);
}

// Wrapper functions for specific time difference calculations
long getTimeDifferenceSeconds(void) {
    return calculateTimeDifference(&currentTime, &lastAlertTime);
}

long getDelayPassedSeconds(uint8_t index) {
    if (index > 3) return 0;
    return calculateTimeDifference(&currentTime, &delayTimes[index]);
}

long getTimeDifferenceMinutes(const TimeComponents *time1, const TimeComponents *time2) {
    return calculateTimeDifference(time1, time2) / 60;
}

long getTimeDifferenceHours(const TimeComponents *time1, const TimeComponents *time2) {
    return calculateTimeDifference(time1, time2) / 3600;
}

long getTimeDifferenceDays(const TimeComponents *time1, const TimeComponents *time2) {
    return calculateTimeDifference(time1, time2) / 86400;
}

// Function to check if a certain number of seconds has passed
uint8_t isSecondMultipleOf(uint32_t seconds) {
	if (seconds > 59)
		return false;
    return (currentTime.rtcTime->Seconds % seconds) == 0 && currentTime.rtcTime->SubSeconds < 10;
}
// Function to check if a certain number of seconds has passed
uint8_t isMinuteMultipleOf(uint32_t minutes) {
	if (minutes > 59)
		return false;
    return (currentTime.rtcTime->Minutes % minutes) == 0 && currentTime.rtcTime->Seconds < 2 && currentTime.rtcTime->SubSeconds < 10;
}
// --- RTC Initialization and Interrupt Handling ---

// Initialize RTC (Call this once in your initialization code)
HAL_StatusTypeDef rtc_init(void) {
    // Assuming hrtc is your RTC_HandleTypeDef
    // Initialize the RTC as needed (e.g., clock source, etc.)
    // This part is highly dependent on your HAL setup
	updateCurrentDateTimeFromRtc();
    return HAL_OK; // Or return HAL_ERROR if initialization fails
}

// RTC Alarm A Callback Function (Override this in your main code)
void HAL_RTC_AlarmAEventCallback(RTC_HandleTypeDef *hrtc) {
    // Handle the RTC Alarm A event here
}

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
