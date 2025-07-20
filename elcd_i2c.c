

#include "elcd_i2c.h"
#include <string.h>

static GPIO_TypeDef *LED_LCD_PORT;                      // LCD LED portu
static uint16_t LED_LCD_PIN;
static I2C_HandleTypeDef *elcdi2c[4];
static uint16_t elcd_address[4];

int8_t elcd_banner;
int16_t ebanner_delay;
int16_t ebanner_percent[6];
uint16_t lcd_counter;

uint16_t lcd_params[32];
char lcd_params_char[8];
static uint8_t lcd_toogle_timer = 30;

LCD_Banner lcd_banner = {
    .count = 0,
    .current_page = 0,
    .page_times = {7, 5, 5, 5},  // Sayfa gösterim süreleri (saniye)
    .last_update = 0,
    .active = 0
};
LCD_State lcd_state = { .init_status = 0, .busy = 0, .error_count = 0,
		.current_col = 0, .current_row = 0, .backlight_state = 1,
		.display_state = 1, .last_command_time = 0, .cursor_state = 0,
		.cursor_blink = 0, .i2c_status = HAL_OK };

void bannerUpdatePageTimes( uint8_t lcd_index ) {
    // Saniye değerlerini hesapla
    int time1 = (ebanner_delay * ebanner_percent[0]) / 100;
    int time2 = (ebanner_delay * ebanner_percent[1]) / 100;
    int time3 = (ebanner_delay * ebanner_percent[2]) / 100;
    int remaining_time = ebanner_delay - (time1 + time2 + time3);
    lcd_banner.page_times[0] = time1;
    lcd_banner.page_times[1] = time2;
    lcd_banner.page_times[2] = time3;
    lcd_banner.page_times[3] = remaining_time;
}
//#static I2C_HandleTypeDef *lcd_i2c; // I2C arayüzü tanımlama

// LCD'ye 4-bit veri yazan yardımcı fonksiyon
static void LCD_Write4Bits(uint8_t lcd_index, uint8_t data, uint8_t rs) {
	if (lcd_state.i2c_status != HAL_OK )
		 return;
    uint8_t high_nibble = (data & 0xF0) | rs | 0x08;  // EN = 1
    uint8_t low_nibble  = ((data << 4) & 0xF0) | rs | 0x08;

    HAL_I2C_Master_Transmit(elcdi2c[lcd_index], elcd_address[lcd_index], &high_nibble, 1, 100);
    HAL_Delay(1);
    high_nibble &= ~0x04; // EN = 0
    HAL_I2C_Master_Transmit(elcdi2c[lcd_index], elcd_address[lcd_index], &high_nibble, 1, 100);

    HAL_I2C_Master_Transmit(elcdi2c[lcd_index], elcd_address[lcd_index], &low_nibble, 1, 100);
    HAL_Delay(1);
    low_nibble &= ~0x04; // EN = 0
    HAL_I2C_Master_Transmit(elcdi2c[lcd_index], elcd_address[lcd_index], &low_nibble, 1, 100);
}

// LCD'ye komut gönderme
void LCD_SendCommand(uint8_t lcd_index, uint8_t cmd) {
	if (lcd_state.init_status != HAL_OK)
		lcd_state.i2c_status = lcd_state.init_status;
	else
		LCD_Write4Bits(lcd_index, cmd, 0);
}

// LCD'ye veri gönderme (karakter yazma)
void LCD_SendData(uint8_t lcd_index, uint8_t data) {
	if (lcd_state.init_status != HAL_OK)
		lcd_state.i2c_status = lcd_state.init_status;
	else
		LCD_Write4Bits(lcd_index, data, 1);
}
HAL_StatusTypeDef LCD_check_connected(uint8_t lcd_index) {
	lcd_state.i2c_status = check_I2C_connected(elcdi2c[lcd_index], elcd_address[lcd_index]);
	if (lcd_state.i2c_status == HAL_OK ) {
		if (LED_LCD_PORT != NULL)
			HAL_GPIO_WritePin(LED_LCD_PORT, LED_LCD_PIN, GPIO_PIN_SET);
		return lcd_state.i2c_status;
		// ok okunca
	} else   if (lcd_state.i2c_status == HAL_ERROR) {
    	if (LED_LCD_PORT != NULL)
    		HAL_GPIO_WritePin(LED_LCD_PORT, LED_LCD_PIN, GPIO_PIN_RESET);
    	return lcd_state.i2c_status;
        // Özel hata işleme
    } else if (lcd_state.i2c_status == HAL_TIMEOUT ) {
    	if (lcd_toogle_timer > 0)
    		lcd_toogle_timer--;
    	if (lcd_toogle_timer == 0) {
    		if (LED_LCD_PORT != NULL)
    			HAL_GPIO_TogglePin(LED_LCD_PORT, LED_LCD_PIN);
    		lcd_toogle_timer = 30;
    	}
    	return lcd_state.i2c_status;
        // Zaman aşımı durumunu
    } else if (lcd_state.i2c_status == HAL_BUSY ) {
    	if (lcd_toogle_timer > 0)
			lcd_toogle_timer--;
		if (lcd_toogle_timer == 0) {
			if (LED_LCD_PORT != NULL)
				HAL_GPIO_TogglePin(LED_LCD_PORT, LED_LCD_PIN);
			lcd_toogle_timer = 10;
		}
    	// Zaman aşımı durumunu
    }
    return lcd_state.i2c_status;
}

// LCD'yi başlat
HAL_StatusTypeDef LCD_Init(I2C_HandleTypeDef *lcdi2c, uint16_t lcd_address, uint8_t lcd_index, GPIO_TypeDef *lport, uint16_t lpin) {
	LED_LCD_PORT = lport;
	LED_LCD_PIN = lpin;
	elcdi2c[lcd_index] = lcdi2c;
	elcd_address[lcd_index] = lcd_address;
	lcd_state.init_status = HAL_ERROR;
	lcd_state.i2c_status = LCD_check_connected(lcd_index);
	if (lcd_state.i2c_status == HAL_OK) {
		HAL_Delay(50);
		LCD_SendCommand(lcd_index, 0x30);
		HAL_Delay(5);
		LCD_SendCommand(lcd_index, 0x30);
		HAL_Delay(1);
		LCD_SendCommand(lcd_index, 0x30);
		HAL_Delay(1);
		LCD_SendCommand(lcd_index, 0x20);  // 4-bit mod
		HAL_Delay(1);

		// LCD yapılandırma
		LCD_SendCommand(lcd_index, LCD_CMD_FUNCTION_SET);
		LCD_SendCommand(lcd_index, LCD_CMD_DISPLAY_OFF);
		LCD_SendCommand(lcd_index, LCD_CMD_CLEAR_DISPLAY);
		LCD_SendCommand(lcd_index, LCD_CMD_ENTRY_MODE_SET);
		LCD_SendCommand(lcd_index, LCD_CMD_DISPLAY_ON);
		lcd_state.init_status = HAL_OK;
		LCD_Clear(lcd_index);
	}
	return lcd_state.init_status;
}

// LCD ekranını temizle
void LCD_Clear(uint8_t lcd_index) {
	if (lcd_state.i2c_status != HAL_OK)
		return;
    LCD_SendCommand(lcd_index, LCD_CMD_CLEAR_DISPLAY);
    HAL_Delay(2);
}

// LCD imlecini başlangıca al
void LCD_Home(uint8_t lcd_index) {
    LCD_SendCommand(lcd_index, LCD_CMD_RETURN_HOME);
    HAL_Delay(2);
}

// LCD'ye string yaz
void LCD_SendString(uint8_t lcd_index, char *str) {
	if (lcd_state.i2c_status != HAL_OK )
		 return;
    while (*str) {
        LCD_SendData(lcd_index, *str++);
    }
}
// LCD'ye string yaz
void LCD_GotoString(uint8_t lcd_index, uint8_t col, uint8_t row, char *str) {
	 if (lcd_state.i2c_status != HAL_OK )
		 return;
	LCD_SetCursor(lcd_index, col, row);
    while (*str) {
        LCD_SendData(lcd_index, *str++);
    }
}
// LCD'de belirli bir satırı tamamen siler
void LCD_ClearRow(uint8_t lcd_index, uint8_t row) {
	if (lcd_state.i2c_status != HAL_OK )
        return;
	char clearString[LCD_DISP_LENGTH + 1]; // Null karakter için +1
	for (uint8_t i = 0; i < LCD_DISP_LENGTH; i++) {
		clearString[i] = ' ';
	}
	clearString[LCD_DISP_LENGTH] = '\0'; // String'i sonlandır
	LCD_GotoString(lcd_index, 0, row, clearString);
}

// LCD'de belirtilen satır ve sütundan başlayarak satırın sonuna kadar siler
void LCD_ClearRowFrom(uint8_t lcd_index, uint8_t col, uint8_t row) {
    if (lcd_state.i2c_status != HAL_OK)
        return;
    LCD_SetCursor(lcd_index, col, row);
    for (uint8_t i = col; i < LCD_DISP_LENGTH; i++) { // Bulunduğumuz sütundan satır sonuna kadar boşluk gönderir (16x2 LCD varsayımı)
        LCD_SendData(lcd_index, ' ');
    }
}
// Belirli bir konuma imleci taşı
void LCD_SetCursor(uint8_t lcd_index, uint8_t col, uint8_t row) {
	if (lcd_state.i2c_status != HAL_OK )
		 return;
    uint8_t address;
    switch (row) {
        case 0: address = LCD_LINE_1 + col; break;
        case 1: address = LCD_LINE_2 + col; break;
        case 2: address = LCD_LINE_3 + col; break;
        case 3: address = LCD_LINE_4 + col; break;
        default: address = LCD_LINE_1 + col;
    }
    LCD_SendCommand(lcd_index, address);
}
void LCD_GotoXY(uint8_t lcd_index, uint8_t col, uint8_t row) {
	LCD_SetCursor(lcd_index, col, row);
}
// LCD aç/kapat fonksiyonları
void LCD_DisplayOn(uint8_t lcd_index)  {
	LCD_SendCommand(lcd_index, LCD_CMD_DISPLAY_ON);
	lcd_state.display_state = 1;
}
void LCD_DisplayOff(uint8_t lcd_index) {
	LCD_SendCommand(lcd_index, LCD_CMD_DISPLAY_OFF);
	lcd_state.display_state = 0;
}
void LCD_CursorOn(uint8_t lcd_index)   {
	LCD_SendCommand(lcd_index, LCD_CMD_CURSOR_ON);
	lcd_state.cursor_state = 0;
}
void LCD_CursorOff(uint8_t lcd_index)  {
	LCD_SendCommand(lcd_index, LCD_CMD_CURSOR_OFF);
	lcd_state.cursor_state = 0;
}

void LCD_BlinkOn(uint8_t lcd_index)    {
	LCD_SendCommand(lcd_index, LCD_CMD_BLINK_ON);
	lcd_state.cursor_blink = 1;
}
void LCD_BlinkOff(uint8_t lcd_index)   {
	LCD_SendCommand(lcd_index, LCD_CMD_BLINK_OFF);
	lcd_state.cursor_blink = 0;
}

// LCD durum raporu oluşturma
void LCD_PrintStatus(uint8_t lcd_index) {
	printf("LCD Status Report:\n");
	printf("Initialized: %s\n", lcd_state.init_status ? "Yes" : "No");
	printf("Errors: %d\n", lcd_state.error_count);
	printf("Position: (%d,%d)\n", lcd_state.current_col, lcd_state.current_row);
	printf("Backlight: %s\n", lcd_state.backlight_state ? "On" : "Off");
	printf("Display: %s\n", lcd_state.display_state ? "On" : "Off");
	printf("I2C Status: %d\n", lcd_state.i2c_status);
	//printf("Last Error: %s\n", LCD_GetErrorString(lcd_state.last_error));
	//printf("Last Operation: %ldms ago\n", lcd_state.last_operation);
	//printf("Queue Size: %d\n", lcd_state.queue_size);
}

// LCD güç yönetimi için yeni fonksiyon
void LCD_PowerSave(uint8_t lcd_index, uint8_t enable) {
//	if (enable) {
//		// Güç tasarrufu modu
//		Queue_AddIT(LCD_CMD, 0x08, 0, 0, NULL);  // Display off
//		Queue_AddIT(LCD_CMD, SET_LCD_BACKLIGHT & ~0x08, 0, 0, NULL); // Backlight off
//		lcd_state.display_state = 0;
//		lcd_state.backlight_state = 0;
//	} else {
//		// Normal mod
//		Queue_AddIT(LCD_CMD, 0x0C, 0, 0, NULL);  // Display on
//		Queue_AddIT(LCD_CMD, SET_LCD_BACKLIGHT, 0, 0, NULL);  // Backlight on
//		lcd_state.display_state = 1;
//		lcd_state.backlight_state = 1;
//	}
}
void LCD_CreateCustomChar(uint8_t lcd_index, uint8_t location, uint8_t charmap[]) {
    // Geçerli indeks aralığında olduğundan emin ol (0-7)
    location &= 0x07;
    // CGRAM adresine geçiş: CGRAM adres komutu 0x40 ile başlar.
    // Her karakter 8 baytlık yer kapladığından (location << 3) kullanıyoruz.
    LCD_SendCommand(lcd_index, 0x40 | (location << 3));
    // 8 satırlık veriyi CGRAM'a yazıyoruz
    for (uint8_t i = 0; i < 8; i++)
    {
         LCD_SendData(lcd_index, LCD_SPECIAL_CHARS[location][i]);
    }
    /*
       Not: Özel karakter tanımlandıktan sonra,
       eğer ekrana veri yazmaya devam edecekseniz,
       DDRAM'a (Display RAM) geri dönmek için uygun bir adres komutu gönderebilirsiniz.
    */
}

// Özel karakterleri yükleme fonksiyonu
void LCD_LoadSpecialChars(uint8_t lcd_index) {
    // CGRAM adresini ayarla
    //Queue_AddIT(LCD_CMD, 0x40, 0, 0, NULL);
    // Her bir karakteri sırayla yükle
    for(uint8_t i = 0; i < LCD_CHAR_MAX && i < 8; i++) {
        for(uint8_t j = 0; j < 8; j++) {
            //Queue_AddIT(LCD_DATA, LCD_SPECIAL_CHARS[i][j], 0, 0, NULL);
        }
    }
}
// Özel karakter yazdırma fonksiyonu
void LCD_WriteSpecialChar(LCD_SpecialChar chr) {
    if(chr < LCD_CHAR_MAX) {
        //Queue_AddIT(LCD_DATA, chr, 0, 0, NULL);
    }
}
void LCD_licence_OK(uint8_t lcd_index) {
	LCD_SpecialChar chr = LCD_CHAR_LISANS_OK; //,     // 🔓 (Kilit açık - Lisanslı)
	LCD_SendData(lcd_index, chr);
	//Queue_AddIT(LCD_DATA, chr, 0, 0, NULL);
}
void LCD_licence_ERROR(uint8_t lcd_index) {
	LCD_SpecialChar chr = LCD_CHAR_LISANS_HATA; //,    // 🔒 (Kilit kapalı - Lisanssız)
	LCD_SendData(lcd_index, chr);
	//Queue_AddIT(LCD_DATA, chr, 0, 0, NULL);
}
void LCD_config_OK(uint8_t lcd_index) {
	LCD_SpecialChar chr = LCD_CHAR_CONFIG_OK; //,     // ⚙️  (Dişli - Konfig Tamam)
	LCD_SendData(lcd_index, chr);
	//Queue_AddIT(LCD_DATA, chr, 0, 0, NULL);
}
void LCD_config_ERROR(uint8_t lcd_index) {
	LCD_SpecialChar chr = LCD_CHAR_CONFIG_HATA; //,    // ⛔ (Yasak - Konfig Hata)
	LCD_SendData(lcd_index, chr);
	//Queue_AddIT(LCD_DATA, chr, 0, 0, NULL);
}
void LCD_WriteInt(uint8_t lcd_index, int value, uint8_t width, bool leading_zeros) {
    char buffer[21];  // LCD maksimum satır uzunluğundan büyük tut
    char format[10];

    // Format belirle: %[0][width]d  (Örneğin: "%05d" veya "%5d")
    snprintf(format, sizeof(format), "%%%s%dd", (leading_zeros ? "0" : ""), width);

    // Formatlı string oluştur
    snprintf(buffer, sizeof(buffer), format, value);

    // LCD'ye yazdır
    LCD_SendString(lcd_index, buffer);
}

void LCD_WriteFloat(uint8_t lcd_index, float value, uint8_t total_width, uint8_t decimal_places, bool leading_zeros) {
    char buffer[21];  // LCD için yeterli uzunlukta buffer
    char format[15];
    // Format belirle: %[0][width].[decimal_places]f (Örneğin: "%07.2f" veya "%7.2f")
    snprintf(format, sizeof(format), "%%%s%d.%df", (leading_zeros ? "0" : ""), total_width, decimal_places);
    // Formatlı string oluştur
    snprintf(buffer, sizeof(buffer), format, value);

    // LCD'ye yazdır
    LCD_SendString(lcd_index, buffer);
}
void LCD_WriteTemperature(uint8_t lcd_index, float temp, bool isCelsius, uint8_t width, uint8_t decimal_places, bool leading_zeros) {
    //char buffer[21];
    // Eğer Fahrenheit istiyorsak, dönüşüm yap
    if (!isCelsius) {
        temp = (temp * 9.0 / 5.0) + 32.0;  // °C -> °F dönüşümü
    }
    // Formatlı sıcaklık yazdır
    LCD_WriteFloat(lcd_index, temp, width, decimal_places, leading_zeros);
    // Sıcaklık birimini ekle
    LCD_SendData(lcd_index, 223); // Derece sembolü (°)
    LCD_SendData(lcd_index, isCelsius ? 'C' : 'F'); // 'C' veya 'F'
}
void LCD_WriteHumidity(uint8_t lcd_index, float humidity, uint8_t width, uint8_t decimal_places, bool leading_zeros) {
    // Formatlı nem değeri yazdır
    LCD_WriteFloat(lcd_index, humidity, width, decimal_places, leading_zeros);
    // Yüzde sembolü ekle
    LCD_SendData(lcd_index, '%');
}


void LCD_DisplayManset(uint8_t lcd_index) {
	if (!lcd_state.init_status || lcd_state.busy)
		return;
	// Üst satıra git
	LCD_GotoXY(lcd_index, 0, 0);
	// 5 dakikada bir program kodu göster, diğer zamanlarda tarih/saat
	if (einit.clocks.eclock.e_dakika % 5 == 0 && einit.clocks.eclock.e_saniye < 30) {
		// Program kodu göster
		char xbuffer[LCD_DISP_LENGTH + 1];
		memset(xbuffer, ' ', LCD_DISP_LENGTH);  // Buffer'ı boşlukla doldur
		xbuffer[LCD_DISP_LENGTH] = '\0';        // String sonlandırıcı
		size_t code_len = strlen(einit.ehardware.version.strcode);
		size_t max_len = LCD_DISP_LENGTH - 3;  // 3 karakter boşluk için
		if (code_len > max_len) {
			code_len = max_len;  // Uzunluğu sınırla
		}
		// Program kodunu kopyala
		strncpy(xbuffer, einit.ehardware.version.strcode, code_len);
		LCD_GotoString(lcd_index, 0, 3, xbuffer);
	} else {
		// Tarih ve saat göster
		char xbuffer[LCD_DISP_LENGTH + 1];
		memset(xbuffer, ' ', LCD_DISP_LENGTH);  // Buffer'ı boşlukla doldur
		xbuffer[LCD_DISP_LENGTH] = '\0';        // String sonlandırıcı
		char buffer[LCD_DISP_LENGTH + 100];
		snprintf(buffer, sizeof(buffer), "%02d-%02d %02d:%02d:%02d ",
				einit.clocks.eclock.e_gun, einit.clocks.eclock.e_ay, einit.clocks.eclock.e_saat,
				einit.clocks.eclock.e_dakika, einit.clocks.eclock.e_saniye);
		size_t code_len = 20;
		strncpy(xbuffer, buffer, code_len);
		LCD_GotoString(lcd_index, 0, 3, xbuffer);
	}
	// Durum göstergeleri
	LCD_GotoXY(lcd_index, 18, 0);
	if (einit.eauthorized == 1) {
		LCD_licence_OK(lcd_index);
	} else {
		LCD_licence_ERROR(lcd_index);
	}
	LCD_GotoXY(lcd_index, 19, 0);
	if (einit.config_status == 0) {
		LCD_config_OK(lcd_index);
	} else {
		LCD_config_ERROR(lcd_index);
	}
}

void LCD_DisplayBanner(uint8_t lcd_index) {
	if (!lcd_state.i2c_status == HAL_OK || !lcd_banner.active) {
		return;
	}
	uint32_t current_time = HAL_GetTick();
	char temp[LCD_DISP_LENGTH + 1];  // 21 karakter (20 + null terminator)
	// Banner başlatma
	if (lcd_banner.count == 0) {
		//LCD_Clear();
		lcd_banner.current_page = 0;
		lcd_banner.count = 1;
		lcd_banner.last_update = current_time;
	}
	// Sayfa değişim zamanı kontrolü
	if (current_time - lcd_banner.last_update
			>= (lcd_banner.page_times[lcd_banner.current_page] * 1000)) {
		lcd_banner.last_update = current_time;
		lcd_banner.current_page = (lcd_banner.current_page + 1) % 4;
		//LCD_Clear();
	}
	// Sayfalara göre içerik gösterimi
	switch (lcd_banner.current_page) {
	case 0:  // Sayfa 1: Versiyon ve ID bilgileri
		LCD_GotoString(lcd_index, 3, 0, "V:");
		LCD_SendString(lcd_index, einit.esoftware.version.strcode);
		char buffer[21];
		snprintf(buffer, sizeof(buffer), "%10lu", einit.ehardware.hw_mcid_reg[0]);
		LCD_GotoString(lcd_index, 0, 1, buffer);
		snprintf(buffer, sizeof(buffer), "%10lu", einit.ehardware.hw_mcid_reg[1]);
		LCD_GotoString(lcd_index, 0, 2, buffer);
		snprintf(buffer, sizeof(buffer), "%10lu", einit.ehardware.hw_mcid_reg[2]);
		LCD_GotoString(lcd_index, 0, 3, buffer);
		snprintf(buffer, sizeof(buffer), "s:%3u", einit.eserial.comm_ID);
		LCD_GotoString(lcd_index, 11, 1, buffer);
		snprintf(buffer, sizeof(buffer), "b:%lu", einit.eserial.handle->Init.BaudRate);
		LCD_GotoString(lcd_index, 11, 2, buffer);
		snprintf(buffer, sizeof(buffer), "p:%ld", einit.eserial.handle->Init.Parity);
		LCD_GotoString(lcd_index, 11, 3, buffer);
		break;
	case 1:  // Sayfa 2: Firma ve sorumlu bilgileri
		//LCD_DisplayManset();  // Üst bilgi bandı
		snprintf(temp, sizeof(temp), "%9llu:", einit.ebanner.ecustomer_no);
		LCD_GotoString(lcd_index, 0, 0, temp);
		LCD_GotoString(lcd_index, 0, 1, einit.ebanner.ecompanyname);
		LCD_GotoString(lcd_index, 0, 2, "Sor:");
		LCD_SendString(lcd_index, einit.ebanner.eresponsible_per);
		LCD_GotoString(lcd_index, 0, 3, "Tek:");
		LCD_SendString(lcd_index, einit.ebanner.etechnical_per);
		break;
	case 2:  // Sayfa 3: Parametre bilgileri
		LCD_GotoXY(lcd_index, 0, 0);
		LCD_DisplayManset(lcd_index);  // Üst bilgi bandı
		char number_buffer[LCD_DISP_LENGTH + 1];  // 21 karakter (20 + null terminator)
		snprintf(temp, sizeof(temp), "%c:", lcd_params_char[0]);
		snprintf(number_buffer, sizeof(number_buffer), "%04hu %04hu %04hu %04hu",
		         lcd_params[1], lcd_params[2], lcd_params[3], lcd_params[4]);
		strncat(temp, number_buffer, LCD_DISP_LENGTH - strlen(temp));
		LCD_GotoString(lcd_index, 0, 1, temp);
		snprintf(temp, sizeof(temp), "%c:", lcd_params_char[1]);
		snprintf(number_buffer, sizeof(number_buffer), "%04hu %04hu %04hu %04hu",
				 lcd_params[5], lcd_params[6], lcd_params[7], lcd_params[8]);
		strncat(temp, number_buffer, LCD_DISP_LENGTH - strlen(temp));
		LCD_GotoString(lcd_index, 0, 2, temp);
		snprintf(temp, sizeof(temp), "%c:", lcd_params_char[2]);
			snprintf(number_buffer, sizeof(number_buffer), "%04hu %04hu %04hu %04hu",
					 lcd_params[9], lcd_params[10], lcd_params[11], lcd_params[12]);
			strncat(temp, number_buffer, LCD_DISP_LENGTH - strlen(temp));
		break;
	case 3:  // Sayfa 4: İletişim bilgileri
		LCD_GotoString(lcd_index, 0, 1, "EVYOS A.S.");
		LCD_GotoString(lcd_index, 0, 2, "www.evyos.com.tr");
		LCD_GotoString(lcd_index, 0, 3, "Tel:4440999");
		break;
	}
}

// Banner başlatma fonksiyonu
void LCD_StartBanner(uint8_t lcd_index) {
	lcd_banner.count = 0;
	lcd_banner.active = 1;
	LCD_DisplayBanner(lcd_index);
}
void LCD_ShowProcessCounter(uint8_t lcd_index, uint8_t col, uint8_t row) {
	char buffer[20];
	snprintf(buffer, sizeof(buffer), "Cnt:%6u ", prg_count);
	LCD_GotoString(lcd_index, col, row, buffer);

}
