/*
 * eprogs.c
 *
 *  Created on: Mar 4, 2025
 *      Author: karatay
 */

#include "eprogs.h"

uint16_t prg_count = 0;
uint8_t process_time = 15;

char sData[254] = { 0 };

EinitType einit;

void prg_add_process(void) {
	//if (einit.clocks.eclock.e_saniye % wait == 0 && ee_saniye_changed()) {
	prg_count++;
		if (prg_count > UINT16_MAX - 10) {
			prg_count = 100;
		}
	//}
}
void e_set_GetSysClockFreq(void) {
	einit.clocks.ClockFreq = HAL_RCC_GetSysClockFreq() / 1000000;
	einit.ecalc_tick_second = (1.0 / einit.clocks.ClockFreq) * 1000; // = 13,889;
}
void format_string(int i, char *buffer, size_t size) {
    snprintf(buffer, size, "Cihaz bulundu: HEX: 0x%02X, DEC: %3d, BIN: ", i, i);

    for (int8_t bit = 7; bit >= 0; bit--) {
        snprintf(buffer + strlen(buffer), size - strlen(buffer), "%d", (i >> bit) & 1);
        if (bit == 1)
            snprintf(buffer + strlen(buffer), size - strlen(buffer), " ");
    }
    snprintf(buffer + strlen(buffer), size - strlen(buffer), "\r\n");
}

HAL_StatusTypeDef check_I2C_connected(I2C_HandleTypeDef *i2c_handle, uint16_t i2c_addr) {
	HAL_StatusTypeDef result;
	uint8_t attempt = 0;
	result = HAL_I2C_IsDeviceReady(i2c_handle, (i2c_addr << 1), 3, HAL_MAX_DELAY);
	if (result == HAL_OK) {
		return HAL_OK;
	} else if (result == HAL_BUSY) {
		while (attempt < 3 && result != HAL_OK) {
			HAL_Delay(100);
			result = HAL_I2C_IsDeviceReady(i2c_handle, (i2c_addr << 1), 3, HAL_MAX_DELAY);
			attempt++;
		}
	}
	return result;
}

char* uint8_to_char_string(const uint8_t *data) {
    // Geçici bir buffer ile başla
    size_t capacity = 8;
    char *buffer = malloc(capacity);
    if (buffer == NULL) {
        return NULL; // malloc başarısız olduysa, NULL döndür
    }
    uint16_t lengthd = 0;
    // Verileri kopyala ve null karakter ekle
    while (data[lengthd] != '\0') {
        if (lengthd == capacity) {
            // Buffer dolarsa, kapasiteyi ikiye katla
            capacity *= 2;
            char *temp = realloc(buffer, capacity);
            if (temp == NULL) {
                free(buffer); // realloc başarısız olduysa, orijinal buffer'ı serbest bırak
                return NULL;
            }
            buffer = temp;
        }
        buffer[lengthd] = (char) data[lengthd];
        lengthd++;
    }
    buffer[lengthd] = '\0';
    return buffer;
}
void get_licence_response(void) {
	//sData_reset();
	if (einit.ebanner.ecustomer_no > 4294967100 ) {
		einit.ebanner.ecustomer_no = 0;
	}
	eDictionary *eSDataDict = malloc(sizeof(eDictionary));
	dict_init(eSDataDict);  // eDictionary dict; kullanmak icin
	dict_add_long(eSDataDict, "Customer_No", einit.ebanner.ecustomer_no);
	char *temp = dict_exportstr(eSDataDict);
	dict_add_str(eSDataDict, "Company", einit.ebanner.ecompanyname);
	temp = dict_exportstr(eSDataDict);
	dict_add_str(eSDataDict, "Aut_per", einit.ebanner.etechnical_per);
	temp = dict_exportstr(eSDataDict);
	dict_add_str(eSDataDict, "Tec_per", einit.ebanner.eresponsible_per);
	temp = dict_exportstr(eSDataDict);
	//char *str1 = dict_exportstr(&edict);
	strcpy(sData, temp);
	strcat(sData,"\n");
	free_edictionary(eSDataDict); // Belleği tamamen temizler UNUTMA!!!!!!!!!!!!!!!!!
	//	sprintf(temp,
	//			"{'config':{'Customer_No':%lld, 'Company':'%s', 'Aut_per':'%s', 'Tec_per':'%s'} }",
	//			einit.ebanner.ecustomer_no, einit.ebanner.ecompanyname,
	//			einit.ebanner.etechnical_per, einit.ebanner.eresponsible_per);
	eSend_Transmit(einit.eserial.handle);
	if (bluetooth_connected == 1) {
		strcpy(sData, temp);
		eSend_Transmit(einit.ebluetooth.handle);
	}

}
void check_init_config(void) {
	einit.config_status = 0;
	if (strlen(einit.ebanner.ecompanyname) < 2) {
		einit.config_status++;
	}
	if (strlen(einit.ebanner.etechnical_per) < 2) {
		einit.config_status += 10;
	}
	if (strlen(einit.ebanner.eresponsible_per) < 2) {
		einit.config_status += 100;
	}
	if (einit.ebanner.ecustomer_no < 1) {
		einit.config_status += 1000;
	}
	if (einit.config_status > 0) {
		memset(sData, '\0', sizeof(sData));
		sprintf(sData, "%s%cGI%c{} %lu\n", const_read_config, const_delimeter,
		const_delimeter, einit.config_status);
		eSend_Transmit(einit.eserial.handle);
		get_licence_response();
	}
}
void eprog_set_config_check(uint8_t delay_minute) {
	if (einit.clocks.eclock.e_dakika % delay_minute == 0
			&& einit.clocks.eclock.e_saniye < 2) {
		if (einit.config_status == 0) {
			check_init_config();
		}
	}
}
void replacechar(char *s, char c1, char c2) {
	int i = 0;
	for (i = 0; s[i]; i++) {
		if (s[i] == c1) {
			s[i] = c2;
		}
	}
}
int32_t get_atoi(char *xval, uint16_t dvalue) {
	int32_t _lc = -1;
	if (xval != NULL) {
		_lc = atoi(xval);
	}
	if (_lc > 0) {
		return _lc;
	}
	return dvalue;
}
void chance_array_marks(char *xlc, char mark_on, char mark_off) {
	replacechar(xlc, mark_on, ' ');
	replacechar(xlc, mark_off, ' ');
}
void trim_array_marks(char *xlc) {
	chance_array_marks(xlc, '[', ']');
}
void trim_dict_marks(char *xlc) {
	chance_array_marks(xlc, '{', '}');
}
void charKaldir(char *string, char mark) {
	int i, j = 0;
	for (i = 0; string[i] != '\0'; ++i) {
		if (string[i] != mark) {
			string[j++] = string[i];
		}
	}
}
void tirnaklariKaldir(char *string) {
	charKaldir(string, '\'');
}

void set_evyos_config_ri(char *xdbid, char *usart_func) {
	check_usart_process = 0;
	if ((strncmp(usart_func, "RI", 2) == 0)) {
		char ye[10] = "False";
		char be[10] = "False";
		if (einit.eauthorized == 1) {
			strcpy(ye, "True");
		}
		if (einit.ebanner.elcd_banner == 1) {
			strcpy(be, "True");
		}
		memset(sData, '\0', sizeof(sData));
		sprintf(sData,
				"%s%c%s%cRI%c[%s, %s, %d, %d, %d, %d, '%s', '%s', '%s', %ld, %ld, %d, '%s', %d, %d ]\n",
				const_recv_config, const_delimeter, xdbid, const_delimeter,
				const_delimeter, ye, be, einit.ebanner.ebanner_delay,
				einit.ebanner.ebanner_percent[0],
				einit.ebanner.ebanner_percent[1],
				einit.ebanner.ebanner_percent[2], einit.ebanner.ecompanyname,
				einit.ebanner.etechnical_per, einit.ebanner.eresponsible_per,
				einit.eperiod_serial_second, einit.eperiod_bluetooth_second,
				einit.esensor_init_second, einit.esoftware.sw_erelase,
				einit.eeprom_enum_id[CONST_EEPROM], einit.eeprom_enum_id[CONGIG_EEPROM_0]);
		check_usart_process = 1;
		eSend_Transmit(einit.eserial.handle);
	}
}

void to_lowercase(char *str) {
	while (*str) {
		*str = tolower((unsigned char )*str);
		str++;
	}
}
uint16_t estr_to_uint16(char *str, uint16_t xdef) {
	trim_array_marks(str);
	trim_dict_marks(str);
	return get_atoi(str, xdef);
}
uint8_t str_to_const_function(char *in_value) {
	strcpy(usart_dbid, str_parser(in_value, const_delimeter, 0));
	strcpy(usart_func, str_parser(in_value, const_delimeter, 1));
	strcpy(usart_data, str_parser(in_value, const_delimeter, 2));
	if (strlen(usart_dbid) > 10) {
		return true;
	}
	return false;
}
HAL_StatusTypeDef einit_serial_recv_it(void) {
	HAL_StatusTypeDef sonuc = HAL_UART_Receive_IT(einit.eserial.handle,
			&seri_data, 1);
//if (sonuc == HAL_OK) {
//	sprintf(sData, "Serial Recv IT HAL_OK");
//	eSend_Transmit(einit.eserial.huart);
//}
	if (sonuc == HAL_ERROR) {
		memset(sData, '\0', sizeof(sData));
		sprintf(sData, "Serial Recv IT HAL_ERROR");
		eSend_Transmit(einit.eserial.handle);
	}
	if (sonuc == HAL_BUSY) {
		memset(sData, '\0', sizeof(sData));
		sprintf(sData, "Serial Recv IT HAL_TIMEOUT");
		eSend_Transmit(einit.eserial.handle);
	}
	if (sonuc == HAL_TIMEOUT) {
		memset(sData, '\0', sizeof(sData));
		sprintf(sData, "Serial Recv IT HAL_TIMEOUT");
		eSend_Transmit(einit.eserial.handle);
	}
	return sonuc;
}
HAL_StatusTypeDef einit_bluetooth_recv_it(void) {
	HAL_StatusTypeDef sonuc = HAL_UART_Receive_IT(einit.ebluetooth.handle,
			&blue_data, 1);
//if (sonuc == HAL_OK) {
//	sprintf(sData, "Bluetooth Recv IT HAL_OK");
//	eSend_Transmit(einit.eserial.huart);
//}
	if (sonuc == HAL_ERROR) {
		memset(sData, '\0', sizeof(sData));
		sprintf(sData, "Bluetooth Recv IT HAL_ERROR");
		eSend_Transmit(einit.eserial.handle);
	}
	if (sonuc == HAL_BUSY) {
		memset(sData, '\0', sizeof(sData));
		sprintf(sData, "Bluetooth Recv IT HAL_TIMEOUT");
		eSend_Transmit(einit.eserial.handle);
	}
	if (sonuc == HAL_TIMEOUT) {
		memset(sData, '\0', sizeof(sData));
		sprintf(sData, "Bluetooth Recv IT HAL_TIMEOUT");
		eSend_Transmit(einit.eserial.handle);
	}
	return sonuc;
}
void evyos_config_wt(char *xdbid, char *xdata, char *usart_func) {
	char *xlc = malloc(512 * sizeof(char)); // Maksimum 512 byte ayır
	if (strncmp(usart_func, "WT", 2) == 0) {
		strcpy(xlc, str_parser(xdata, ",", 0));
		uint16_t y = estr_to_uint16(xlc, einit.clocks.eclock.e_yil);
		if (y > 2000) {
			y = y - 2000;
		}
		einit.clocks.eclock.e_yil = y;
		strcpy(xlc, str_parser(xdata, ",", 1));
		einit.clocks.eclock.e_ay = estr_to_uint16(xlc, einit.clocks.eclock.e_ay);
		if (einit.clocks.eclock.e_ay > 12) {
			einit.clocks.eclock.e_ay = 1;
		}
		strcpy(xlc, str_parser(xdata, ",", 2));
		einit.clocks.eclock.e_gun = estr_to_uint16(xlc, einit.clocks.eclock.e_gun);
		if (einit.clocks.eclock.e_gun > 31) {
			einit.clocks.eclock.e_gun = 1;
		}
		strcpy(xlc, str_parser(xdata, ",", 3));
		einit.clocks.eclock.e_hafta = estr_to_uint16(xlc, einit.clocks.eclock.e_hafta);
		if (einit.clocks.eclock.e_hafta > 7) {
			einit.clocks.eclock.e_hafta = 1;
		}
		strcpy(xlc, str_parser(xdata, ",", 4));
		einit.clocks.eclock.e_saat = estr_to_uint16(xlc, einit.clocks.eclock.e_saat);
		if (einit.clocks.eclock.e_saat > 23) {
			einit.clocks.eclock.e_saat = 1;
		}
		strcpy(xlc, str_parser(xdata, ",", 5));
		einit.clocks.eclock.e_dakika = estr_to_uint16(xlc, einit.clocks.eclock.e_dakika);
		if (einit.clocks.eclock.e_dakika > 59) {
			einit.clocks.eclock.e_dakika = 1;
		}
		strcpy(xlc, str_parser(xdata, ",", 6));
		einit.clocks.eclock.e_saniye = estr_to_uint16(xlc, einit.clocks.eclock.e_saniye);
		if (einit.clocks.eclock.e_saniye > 59) {
			einit.clocks.eclock.e_saniye = 1;
		}
//		set_date(einit.clocks.eclock.e_yil, einit.clocks.eclock.e_ay, einit.clocks.eclock.e_gun,
//				einit.clocks.eclock.e_hafta, einit.clocks.eclock.e_saat,
//				einit.clocks.eclock.e_dakika, einit.clocks.eclock.e_saniye);
	}
	if (einit.clocks.eclock.e_yil < 2) {
		memset(sData, '\0', sizeof(sData));
		sprintf(sData, "%s%c%s%cRT%c", const_recv_config, const_delimeter, xdbid, const_delimeter,
		const_delimeter);
		eSend_Transmit(einit.eserial.handle);
	}
}

uint8_t check_int(char *xv, uint8_t xdef) {
	if (strlwr(xv)[0] == 't') {
		return 1;
	}
	if (strlwr(xv)[0] == '1') {
		return 1;
	}
	if (strlwr(xv)[0] == 'f') {
		return 0;
	}
	if (strlwr(xv)[0] == '0') {
		return 0;
	}
	return xdef;
}


void set_config_dict(char *xdbid, char *xdata, char *usart_func) {
	if (xdata != NULL) {
		uint8_t i = 0;
		uint8_t b = 1;
		char *xlc = malloc(512 * sizeof(char)); // Maksimum 512 byte ayır
		if (xlc == NULL) {
			fprintf(stderr, "Bellek tahsis edilemedi!\n");
			free(xlc);
			return;
		}

		while (b == 1) {
			char *xlc0 = malloc(512 * sizeof(char)); // Maksimum 512 byte ayır
			char *xlc1 = malloc(512 * sizeof(char)); // Maksimum 512 byte ayır
			strcpy(xlc, str_parser(xdata, ",", i));
			xlc[511] = '\0'; // Son karakteri güvenli hale getir
			if (xlc == NULL) {
				if (i < 1) {
					memset(sData, '\0', sizeof(sData));
					sprintf(sData,
							"'Msg': 'You sent incorrect data for your config values..%s' ",
							xdata);
					eSend_Transmit(einit.eserial.handle);
				} else {
					b = 9;
				}
			} else {
				trim_dict_marks(xlc);
				tirnaklariKaldir(xlc);
				memset(xlc, 0, strlen(xlc));
				i++;
				strcpy(xlc0, str_parser(xlc, ":", 0));
				//char *xlc0 = strlwr(str_parser(xlc, ":", 0));
				memset(xlc1, 0, strlen(xlc1));
				strcpy(xlc1, str_parser(xlc, ":", 1));
				if (xlc1 != NULL) {
					char src[20];
					char dest[21];
					if (strncmp(xlc0, "authorized", 10) == 0) {
						einit.eauthorized = check_int(xlc1, einit.eauthorized);
					}
					if (strncmp(xlc0, "lcd_banner", 6) == 0) {
						einit.ebanner.elcd_banner = check_int(xlc1,
								einit.ebanner.elcd_banner);
					}
					if (strncmp(xlc0, "banner_delay", 12) == 0) {
						einit.ebanner.ebanner_delay = get_atoi(xlc1,
								einit.ebanner.ebanner_delay);
					}
					if (strncmp(xlc0, "banner_page1", 12) == 0) {
						einit.ebanner.ebanner_percent[0] = get_atoi(xlc1,
								einit.ebanner.ebanner_percent[0]);
					}
					if (strncmp(xlc0, "banner_page2", 12) == 0) {
						einit.ebanner.ebanner_percent[1] = get_atoi(xlc1,
								einit.ebanner.ebanner_percent[1]);
					}
					if (strncmp(xlc0, "banner_page3", 12) == 0) {
						einit.ebanner.ebanner_percent[2] = get_atoi(xlc1,
								einit.ebanner.ebanner_percent[2]);
					}
					if (strncmp(xlc0, "serial_second", 13) == 0) {
						einit.eperiod_serial_second = get_atoi(xlc1,
								einit.eperiod_serial_second);
					}
					if (strncmp(xlc0, "bluetooth_second", 16) == 0) {
						einit.eperiod_serial_second = get_atoi(xlc1,
								einit.eperiod_serial_second);
					}
					if (strncmp(xlc0, "sensor_init_second", 19) == 0) {
						einit.esensor_init_second = get_atoi(xlc1,
								einit.esensor_init_second);
					}
					if (strncmp(xlc0, "company_name", 12) == 0) {
						memset(dest, '\0', sizeof(dest));
						tirnaklariKaldir(xlc1);
						strcpy(src, xlc1);
						strncpy(dest, src, 20);
						strncpy(einit.ebanner.ecompanyname, dest, 21);
					}
					if (strncmp(xlc0, "technical_phone", 15) == 0) {
						memset(dest, '\0', sizeof(dest));
						tirnaklariKaldir(xlc1);
						strcpy(src, xlc1);
						strncpy(dest, src, 20);
						strncpy(einit.ebanner.etechnical_per, dest, 21);
					}
					if (strncmp(xlc0, "responsible_phone", 17) == 0) {
						memset(dest, '\0', sizeof(dest));
						tirnaklariKaldir(xlc1);
						strcpy(src, xlc1);
						strncpy(dest, src, 20);
						strncpy(einit.ebanner.eresponsible_per, dest, 21);
					}
				}
			}
		}
		free(xlc);
		return;
	}
	memset(sData, '\0', sizeof(sData));
	sprintf(sData, "'Msg': 'You sent incorrect  for your config values...' \n");
	eSend_Transmit(einit.eserial.handle);
}

void set_config_array(char *xdbid, char *xdata, char *usart_func) {

	if (xdata != NULL) {
		uint8_t i = 0;
		if (strlen(xdata) > 10) {
			char *xlc = malloc(512 * sizeof(char));
			strcpy(xlc, str_parser(xdata, ",", i));
			if (xlc == NULL) {
				if (i < 1) {
					memset(sData, '\0', sizeof(sData));
					sprintf(sData,
							"'Msg': 'You sent incorrect data for your config values..%s' ",
							xdata);
					eSend_Transmit(einit.eserial.handle);
				}
			} else {
				trim_array_marks(xdata);
				memset(sData, '\0', sizeof(sData));
				char src[20]; // "1, 1, 10, 20, 20, 30, Gunes Apt 151, 0532 123 45 67, , 15, 300 \\n!"
				char dest[21];
				strcpy(xlc, str_parser(xdata, ",", 0));
				einit.eauthorized = check_int(xlc, einit.eauthorized);
				strcpy(xlc, str_parser(xdata, ",", 1));
				einit.ebanner.elcd_banner = check_int(xlc,
						einit.ebanner.elcd_banner);
				strcpy(xlc, str_parser(xdata, ",", 2));
				einit.ebanner.ebanner_delay = get_atoi(xlc,
						einit.ebanner.ebanner_delay);
				strcpy(xlc, str_parser(xdata, ",", 3));
				einit.ebanner.ebanner_percent[0] = get_atoi(xlc,
						einit.ebanner.ebanner_percent[0]);
				strcpy(xlc, str_parser(xdata, ",", 4));
				einit.ebanner.ebanner_percent[1] = get_atoi(xlc,
						einit.ebanner.ebanner_percent[1]);
				strcpy(xlc, str_parser(xdata, ",", 5));
				einit.ebanner.ebanner_percent[2] = get_atoi(xlc,
						einit.ebanner.ebanner_percent[2]);
				strcpy(xlc, str_parser(xdata, ",", 6));
				memset(dest, '\0', sizeof(dest));
				tirnaklariKaldir(xlc);
				strcpy(src, xlc);
				strncpy(dest, src, 20);
				strncpy(einit.ebanner.ecompanyname, dest, 21);
				strcpy(xlc, str_parser(xdata, ",", 7));
				memset(dest, '\0', sizeof(dest));
				tirnaklariKaldir(xlc);
				strcpy(src, xlc);
				strncpy(dest, src, 20);
				strncpy(einit.ebanner.etechnical_per, dest, 21);
				strcpy(xlc, str_parser(xdata, ",", 8));
				memset(dest, '\0', sizeof(dest));
				tirnaklariKaldir(xlc);
				strcpy(src, xlc);
				strncpy(dest, src, 20);
				strncpy(einit.ebanner.eresponsible_per, dest, 21);
				strcpy(xlc, str_parser(xdata, ",", 9));
				einit.eperiod_serial_second = get_atoi(xlc,
						einit.eperiod_serial_second);
				strcpy(xlc, str_parser(xdata, ",", 10));
				einit.eperiod_bluetooth_second = get_atoi(xlc,
						einit.eperiod_bluetooth_second);
				strcpy(xlc, str_parser(xdata, ",", 11));
				einit.esensor_init_second = get_atoi(xlc,
						einit.esensor_init_second);
				free(xlc);
			}
		}
	}
	//authorized:%s, banner:%s, banner_delay:%d, banner_page1:%d, banner_page2:%d, company_name:'%s', technical_phone:'%s', responsible_phone:'%s' }
}

void send_nlg_db_close(const char *dbid, const char *xmsg, const char *xdetail) {
	memset(sData, '\0', sizeof(sData));
	sprintf(sData, "%s%c%s%c{%s} %c %s", const_recv_close, const_delimeter,
			dbid, const_delimeter, xmsg, const_delimeter, xdetail);
	eSend_Transmit(einit.eserial.handle);
}
void evyos_config_wi_gi(char *xdbid, char *xdata, char *usart_func) {
	check_usart_process = 0;
	if ((strncmp(usart_func, "WI", 2) == 0)
			|| (strncmp(usart_func, "GI", 2) == 0)) {
		if (strlen(xdata) > 0) {
			if (xdata != NULL) {
				if (*xdata == '{') {
					set_config_dict(xdbid, xdata, usart_func);
				} else {
					set_config_array(xdbid, xdata, usart_func);
				}
				send_nlg_db_close(xdbid, "", xdata);
				check_usart_process = 1;
			}
		}
	}
}
void check_modbus_params(int32_t ebaudrate) {
	if (einit.emodbus[0].ebaudRate < 1) {
		einit.emodbus[0].ebaudRate = ebaudrate;
	}
	if (einit.emodbus[0].eparity != 'E' && einit.emodbus[0].eparity != 'O') {
		einit.emodbus[0].eparity = 'N';
	}
	if (einit.emodbus[0].edataBits < 1) {
		einit.emodbus[0].edataBits = 8;
	}
	if (einit.emodbus[0].estopBits < 1) {
		einit.emodbus[0].estopBits = 1;
	}
	if (einit.emodbus[0].etimeout < 1) {
		einit.emodbus[0].etimeout = 1000;
	}
	if (einit.emodbus[0].eretryCount < 1) {
		einit.emodbus[0].eretryCount = 3;
	}
	if (einit.emodbus[0].eportNumber < 1) {
		einit.emodbus[0].eportNumber = const_port_number;
	}
	if (einit.emodbus[0].eipAddress[0] < 1) {
		strcpy(einit.emodbus[0].eipAddress, const_ip_address);
	}
}
HAL_StatusTypeDef UpdateUARTConfig(UART_HandleTypeDef *huart, SetEusartType econfig) {
	// UART Instance'ı ayarla
	// huart->Instance = USARTx;
	// Baud Rate
	huart->Init.BaudRate = econfig.ebaudRate;
	// Word Length (Data Bits)
	if (econfig.edataBits == 9) {
		huart->Init.WordLength = UART_WORDLENGTH_9B;
	} else {
		huart->Init.WordLength = UART_WORDLENGTH_8B;
	}
	// Stop Bits
	if (econfig.estopBits == 1) {
		huart->Init.StopBits = UART_STOPBITS_1;
	} else if (econfig.estopBits == 2) {
		huart->Init.StopBits = UART_STOPBITS_2;
	}
	// Parity
	if (econfig.eparity == 'N') {
		huart->Init.Parity = UART_PARITY_NONE;
	} else if (econfig.eparity == 'E') {
		huart->Init.Parity = UART_PARITY_EVEN;
	} else if (econfig.eparity == 'O') {
		huart->Init.Parity = UART_PARITY_ODD;
	}
	// Mode (Transmit and Receive)
	huart->Init.Mode = UART_MODE_TX_RX;
	// Hardware Flow Control (default None)
	huart->Init.HwFlowCtl = UART_HWCONTROL_NONE;
	// OverSampling (default 16)
	huart->Init.OverSampling = UART_OVERSAMPLING_16;
	// UART Init
	HAL_StatusTypeDef xresult = HAL_UART_Init(huart);
//    if (xresult != HAL_OK) {
//        // Hata durumunda işlem yapabilirsiniz
//        printf("UART Init Error!\n");
//    } else {
//        printf("UART Configured Successfully for USART%x!\n", (int)((uint32_t)USARTx - (uint32_t)USART1) / 0x400);
//    }
	return xresult;
}

void show_serial_ports(void) {
	uint8_t sakla = einit.eauthorized;
	einit.eauthorized = 1;
	sprintf(sData, " ");
	eSend_Transmit(einit.eserial.handle);
	sprintf(sData,
			"\n***********************************************************************\n");
	eSend_Transmit(einit.eserial.handle);
	sprintf(sData, "{'Msg':'Start Serial Application.'}\n");
	eSend_Transmit(einit.eserial.handle);
//	if (bluetooth_connected == 1) {
//		sprintf(sData, "{'Msg':'Start Bluetooth Application.'}");
//		eSend_Transmit(einit.ebluetooth.handle);
//	}
	if (einit.eserial.usart != NULL ) {
		sprintf(sData,
				"{'config':{'UART':'SS', 'BaudRate':%lu, 'StopBits':%lu, 'Parity':%lu }}\n",
				einit.eserial.handle->Init.BaudRate,
				einit.eserial.handle->Init.StopBits,
				einit.eserial.handle->Init.Parity);
		eSend_Transmit(einit.eserial.handle);
	}
	if (einit.ebluetooth.usart != NULL ) {
		sprintf(sData,
				"{'config':{'UART':'SB', 'BaudRate':%lu, 'StopBits':%lu, 'Parity':%lu }}\n",
				einit.ebluetooth.handle->Init.BaudRate,
				einit.ebluetooth.handle->Init.StopBits,
				einit.ebluetooth.handle->Init.Parity);
		eSend_Transmit(einit.eserial.handle);
	}
	if (einit.emodbus[0].usart != NULL ) {
		sprintf(sData,
				"{'config':{'UART':'SM', ''slaveID':%u, 'BaudRate':%lu, 'StopBits':%lu, 'Parity':%lu }}\n", //\r\n',
				einit.emodbus[0].Slave_ID,
				einit.emodbus[0].handle->Init.BaudRate,
				einit.emodbus[0].handle->Init.StopBits,
				einit.emodbus[0].handle->Init.Parity);
		eSend_Transmit(einit.eserial.handle);
	}
	einit.eauthorized = sakla;
}


