#ifndef LCD_I2C_H_
#define LCD_I2C_H_

#include "stm32f3xx.h"
#include "eprogs.h"
#include <stdbool.h>

#define SLAVE_ADDRESS_LCD1 0x27 // HEX: 0x27, DEC:  39, BIN: 0010011
#define SLAVE_ADDRESS_LCD2 0x7E
//extern HAL_StatusTypeDef elcd_status;

extern uint16_t lcd_counter;
extern uint16_t lcd_params[32];
extern char lcd_params_char[8];
extern int8_t elcd_banner;
extern int16_t ebanner_delay;
extern int16_t ebanner_percent[6];
//extern char lcd_buffer[LCD_BUFFER_SIZE];
#define LCD_DISP_LENGTH 20

// Banner Yapısı
typedef struct {
    uint16_t count;

    uint16_t current_page;
    uint16_t page_times[4];
    uint32_t last_update;
    uint8_t active;
} LCD_Banner;

extern LCD_Banner lcd_banner;

// LCD Durum Yapısı
typedef struct {
    uint8_t init_status;         // LCD başlatma durumu
    uint8_t busy;                // Meşgul durumu
    uint8_t error_count;         // Hata sayacı
    uint8_t current_col;         // Mevcut sütun pozisyonu
    uint8_t current_row;         // Mevcut satır pozisyonu
    uint8_t backlight_state;     // Arka ışık durumu (1:açık, 0:kapalı)
    uint8_t display_state;       // Ekran durumu (1:açık, 0:kapalı)
    uint32_t last_command_time;  // Son komut zamanı
    uint8_t cursor_state;        // Cursor durumu
    uint8_t cursor_blink;        // Cursor yanıp sönme durumu
    HAL_StatusTypeDef i2c_status; // I2C haberleşme durumu
} LCD_State;

// Global değişken olarak tanımlama
extern LCD_State lcd_state;

typedef enum  {
	eLCD_0,
	eLCD_1,
	eLCD_2,
	eLCD_3,
	eLCD_4,
	eLCD_5,
	eLCD_6,
	eLCD_7
} eLCD_NO;
// LCD I2C Adresi (PCF8574 için)
//#define LCD_I2C_ADDRESS 0x4E  // veya 0x7E (cihaz adresine göre değişebilir)
typedef enum {
    LCD_CHAR_DERECE = 0,    // °  (Derece işareti)
    LCD_CHAR_SANTIGRAT,     // °C (Santigrat)
    LCD_CHAR_OK,            // ✓  (Onay işareti)
    LCD_CHAR_GULEN_YUZ,     // ☺  (Gülen yüz)
    LCD_CHAR_UZGUN_YUZ,     // ☹  (Üzgün yüz)
    LCD_CHAR_CALISMA,       // ↻  (Çalışıyor/Yükleniyor)
    LCD_CHAR_BEKLE,         // ⌛ (Kum saati)
    LCD_CHAR_BEKLE_1,       // ⧖  (Kum saati animasyon 1)
    LCD_CHAR_BEKLE_2,       // ⧗  (Kum saati animasyon 2)
    LCD_CHAR_HATA,          // !  (Ünlem/Hata işareti)
    LCD_CHAR_OK_UP,         // ↑  (Yukarı ok)
    LCD_CHAR_OK_DOWN,       // ↓  (Aşağı ok)
    LCD_CHAR_SAAT,          // ⌚ (Saat)
    LCD_CHAR_TAKVIM,        // 📅 (Takvim)
    LCD_CHAR_LISANS_OK,     // 🔓 (Kilit açık - Lisanslı)
    LCD_CHAR_LISANS_HATA,   // 🔒 (Kilit kapalı - Lisanssız)
    LCD_CHAR_CONFIG_OK,     // ⚙️  (Dişli - Konfig Tamam)
    LCD_CHAR_CONFIG_HATA,   // ⛔ (Yasak - Konfig Hata)
    LCD_CHAR_MAX           // Toplam karakter sayısı
} LCD_SpecialChar;

static const uint8_t LCD_SPECIAL_CHARS[LCD_CHAR_MAX][8] = {
    [LCD_CHAR_DERECE] = { 0b00110, 0b01001, 0b01001, 0b00110, 0b00000, 0b00000, 0b00000, 0b00000 },
    [LCD_CHAR_SANTIGRAT] = { 0b01000, 0b10100, 0b01000, 0b00011, 0b00100, 0b00100, 0b00011, 0b00000 },
    [LCD_CHAR_OK] = { 0b00000, 0b00000, 0b00001, 0b00010, 0b10100, 0b01000, 0b00000, 0b00000 },
    [LCD_CHAR_GULEN_YUZ] = { 0b00000, 0b00000, 0b01010, 0b00000, 0b10001, 0b01110, 0b00000, 0b00000 },
    [LCD_CHAR_UZGUN_YUZ] = { 0b00000, 0b00000, 0b01010, 0b00000, 0b01110, 0b10001, 0b00000, 0b00000 },
    [LCD_CHAR_CALISMA] = { 0b00000, 0b01110, 0b11111, 0b10001, 0b10001, 0b01010, 0b00100, 0b00000 },
    [LCD_CHAR_BEKLE] = { 0b11111, 0b11111, 0b01110, 0b00100, 0b01110, 0b11111, 0b11111, 0b00000 },
    [LCD_CHAR_BEKLE_1] = { 0b11111, 0b10001, 0b01110, 0b00100, 0b01110, 0b10001, 0b11111, 0b00000 },
    [LCD_CHAR_BEKLE_2] = { 0b11111, 0b11111, 0b01110, 0b00100, 0b01110, 0b11111, 0b11111, 0b00000 },
    [LCD_CHAR_HATA] = { 0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b00000, 0b00100, 0b00000 },
    [LCD_CHAR_OK_UP] = { 0b00100, 0b01110, 0b10101, 0b00100, 0b00100, 0b00100, 0b00000, 0b00000 },
    [LCD_CHAR_OK_DOWN] = { 0b00000, 0b00100, 0b00100, 0b00100, 0b10101, 0b01110, 0b00100, 0b00000 },
	[LCD_CHAR_SAAT] = { 0b00000, 0b01110, 0b10101, 0b10111, 0b10001, 0b01110, 0b00000, 0b00000 },
	[LCD_CHAR_TAKVIM] = { 0b11111, 0b10001, 0b11111, 0b10001, 0b10101, 0b10001, 0b11111, 0b00000 },
	[LCD_CHAR_LISANS_OK] = { 0b01110, 0b10001, 0b10001, 0b11111, 0b11011, 0b11011, 0b11111, 0b00000 },
	[LCD_CHAR_LISANS_HATA] = { 0b01110, 0b10001, 0b10001, 0b11111, 0b11111, 0b11111, 0b11111, 0b00000 },
	[LCD_CHAR_CONFIG_OK] = { 0b01110, 0b11111, 0b10001, 0b01110, 0b01110, 0b10001, 0b11111, 0b01110 },
	[LCD_CHAR_CONFIG_HATA] = { 0b00000, 0b01110, 0b11011, 0b10001, 0b10001, 0b11011, 0b01110, 0b00000 }
};
// LCD Komutları
#define LCD_CMD_CLEAR_DISPLAY   0x01
#define LCD_CMD_RETURN_HOME     0x02
#define LCD_CMD_ENTRY_MODE_SET  0x06
#define LCD_CMD_DISPLAY_ON      0x0C
#define LCD_CMD_DISPLAY_OFF     0x08
#define LCD_CMD_CURSOR_ON       0x0E
#define LCD_CMD_CURSOR_OFF      0x0C
#define LCD_CMD_BLINK_ON        0x0D
#define LCD_CMD_BLINK_OFF       0x0C
#define LCD_CMD_FUNCTION_SET    0x28
#define LCD_CMD_SET_CGRAM       0x40
#define LCD_CMD_SET_DDRAM       0x80

// LCD Satır Tanımları
#define LCD_LINE_1  0x80
#define LCD_LINE_2  0xC0
#define LCD_LINE_3  0x94
#define LCD_LINE_4  0xD4

// LCD Sürücü Fonksiyonları
HAL_StatusTypeDef LCD_Init(I2C_HandleTypeDef *lcdi2c, uint16_t lcd_address, uint8_t lcd_index, GPIO_TypeDef *lport, uint16_t lpin);
void LCD_SendCommand(uint8_t lcd_index, uint8_t cmd);
void LCD_SendData(uint8_t lcd_index, uint8_t data);
void LCD_SendString(uint8_t lcd_index, char *str);
void LCD_GotoString(uint8_t lcd_index, uint8_t col, uint8_t row, char *str);
void LCD_SetCursor(uint8_t lcd_index, uint8_t col, uint8_t row);
void LCD_Clear(uint8_t lcd_index );
void LCD_Home(uint8_t lcd_index );
void LCD_DisplayOn(uint8_t lcd_index );
void LCD_DisplayOff(uint8_t lcd_index );
void LCD_CursorOn(uint8_t lcd_index );
void LCD_CursorOff(uint8_t lcd_index );
void LCD_BlinkOn(uint8_t lcd_index);
void LCD_BlinkOff(uint8_t lcd_index);
void LCD_WriteInt(uint8_t lcd_index, int value, uint8_t width, bool leading_zeros);
void LCD_WriteFloat(uint8_t lcd_index, float value, uint8_t total_width, uint8_t decimal_places, bool leading_zeros);
void LCD_WriteTemperature(uint8_t lcd_index, float temp, bool isCelsius, uint8_t width, uint8_t decimal_places, bool leading_zeros);
void LCD_WriteHumidity(uint8_t lcd_index, float humidity, uint8_t width, uint8_t decimal_places, bool leading_zeros);
HAL_StatusTypeDef LCD_check_connected(uint8_t lcd_index);
void bannerUpdatePageTimes( uint8_t lcd_index );
void LCD_StartBanner(uint8_t lcd_index);
void LCD_DisplayBanner(uint8_t lcd_index);
void LCD_ShowProcessCounter(uint8_t lcd_index, uint8_t col, uint8_t row);
void LCD_ClearRowFrom(uint8_t lcd_index, uint8_t col, uint8_t row);
void LCD_ClearRow(uint8_t lcd_index, uint8_t row);


#endif // LCD_I2C_H_
