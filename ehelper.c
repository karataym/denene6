
#include "ehelper.h"
#include <stdarg.h>

// Global değişkenler
UART_HandleTypeDef *debug_huart = NULL;
static char project_name[32];
uint8_t custom_mac_address[MAC_ADDRESS_LENGTH];

#define ADDR_ID1   0x1FFF7590
#define ADDR_ID2   0x1FFF7594
#define ADDR_ID3   0x1FFF7598
#define custom_file_mac 0

typedef struct {
    uint32_t ID1;
    uint32_t ID2;
    uint32_t ID3;
} UniqueID_TypeDef;

//UniqueID_TypeDef* uniqueID = (UniqueID_TypeDef*)ADDR_ID1;

//extern ETH_HandleTypeDef heth;
//extern struct netif gnetif;  // lwip.c'den
/**
 * @brief Helper kütüphanesini başlatır
 */
void helper_init(const char *name, UART_HandleTypeDef *huart)
{
    // Proje ismini kaydet
    strncpy(project_name, name, sizeof(project_name)-1);
    project_name[sizeof(project_name)-1] = '\0';
    
//    // Debug UART'ı ayarla
    debug_huart = huart;
//    generate_unique_mac(); // unigue edm macid
//    // Başlangıç mesajı
    if (custom_mac_address[0] != 0) {
    printf("\r\n=== %s Starting... === %02X:%02X:%02X:%02X:%02X:%02X\r\n", project_name,
    		custom_mac_address[0], custom_mac_address[1], custom_mac_address[2],
			custom_mac_address[3], custom_mac_address[4], custom_mac_address[5]);
    } else {
    	printf("\r\n=== %s Starting... ===\r\n", project_name);
    }
//    gnetif.hwaddr_len = MAC_ADDRESS_LENGTH; // MAC adresi 6 bayttır
//	memcpy(gnetif.hwaddr, custom_mac_address, MAC_ADDRESS_LENGTH); // MAC adresini kopyala
//	memcpy(heth.Init.MACAddr, custom_mac_address, MAC_ADDRESS_LENGTH);
}

/**
 * @brief printf için UART write fonksiyonu
 */
int uart_write(char *ptr, int len)
{
    if (debug_huart != NULL) {
        HAL_UART_Transmit(debug_huart, (uint8_t*)ptr, len, HAL_MAX_DELAY);
    }
    return len;
}

/**
 * @brief printf için syscalls _write fonksiyonu
 */
int _write(int file, char *ptr, int len)
{
    return uart_write(ptr, len);
} 

uint8_t string_to_mac(const char *mac_str, uint8_t *mac) {
    // Hata kontrolleri
    if (mac_str == NULL || mac == NULL) {
        printf("Hata: Gecersiz parametreler!\n");
        return 0;
    }
    if (strlen(mac_str) != 17) {
        printf("Hata: Gecersiz MAC adresi uzunlugu!\n");
        return 0;
    }
    // Gecersiz karakter kontrolü
    for (int i = 0; i < strlen(mac_str); ++i)
    {
        if (i % 3 != 2 && !isxdigit((unsigned char)mac_str[i])) // : olmayan karakterler hex değilse
        {
            printf("Hata: Gecersiz karakter!\n");
            return 0;
        } else if (i % 3 == 2 && mac_str[i] != ':') // : olması gereken yerde : yoksa
        {
            printf("Hata: Gecersiz ayirac!\n");
            return 0;
        }
    }
    for (int i = 0; i < MAC_ADDRESS_LENGTH; i++) {
        if (sscanf(mac_str + i * 3, "%02hhX", &mac[i]) != 1) {
            printf("Hata: Hexadecimal donusumu basarisiz!\n");
            return 0; // Dönüşüm başarısız oldu
        }
    }
    return 1; // Başarılı dönüşüm
}
void mac_to_string(const uint8_t mac[6], char *str) {
    // MAC adresini "XX:XX:XX:XX:XX:XX" formatında string'e çevir
    sprintf(str, "%02X:%02X:%02X:%02X:%02X:%02X",
            mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}
bool mac_to_string_1(const uint8_t mac[MAC_ADDRESS_LENGTH], char *str, size_t str_size) {
     // Hata kontrolleri
    if (mac == NULL || str == NULL) {
        printf("Hata: Gecersiz parametreler!\n");
        return false;
    }
     if (str_size < MAC_ADDRESS_STRING_LENGTH) {
        printf("Hata: Yetersiz bellek!\n");
        return false;
    }
    // MAC adresini "XX:XX:XX:XX:XX:XX" formatında string'e çevir
    int ret = snprintf(str, str_size, "%02X:%02X:%02X:%02X:%02X:%02X",
            mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    if(ret < 0 || ret >= str_size){
        printf("Hata: String e yazma başarısız!\n");
        return false;
    }
    return true;
}

uint8_t is_valid_mac(const char *mac) {
    // MAC adresi formatı: XX:XX:XX:XX:XX:XX (6 byte, 17 karakter)
    if (strlen(mac) != 17) { return 0;   }
    // Her iki karakter arasında ':' olmalı
    for (int i = 2; i < 17; i += 3) {
        if (mac[i] != ':') { return 0; }
    }
    // Karakterler hexadecimale uygun olmalı ve değerleri kontrol et
    for (int i = 0; i < 17; i += 3) {
        if (!isxdigit((unsigned char)mac[i]) || !isxdigit((unsigned char)mac[i + 1])) {
            return 0;
        }
    }
    // İlk oktetin değerini kontrol et (Unicast ve Multicast)
    // İlk okteti integer'a dönüştür
    int first_octet;
    sscanf(mac, "%x", &first_octet);
    // Unicast adresler için en düşük anlamlı bit 0 olmalı
    if ((first_octet & 0x01) != 0) {
        // Multicast adres
        // Multicast adresler için özel bir kontrol yapılması gerekebilir.
        // İhtiyacınıza göre buraya ek kontroller ekleyebilirsiniz.
    } else {
        // Unicast adres
    }
     // Yerel olarak atanan adresler için ikinci en düşük anlamlı bit 1 olmalı (isteğe bağlı)
    //if ((first_octet & 0x02) == 0x02) {
    //    // Yerel olarak atanan adres
    // } else {
    //     //Evrensel olarak atanan adres (Genel kullanılan)
    // }
    return 1;
}
void print_mac(char *mac_str) {
	char who_adr[30] = {0};
	 if (is_valid_mac(mac_str)) {
	        printf("%s gecerli bir MAC adresidir.\n", mac_str);
	        if (is_local_mac(mac_str)) {
	        	strcpy(who_adr,"YEREL");
	            //printf("%s yerel (local) bir MAC adresidir.\n", mac_str);
	        } else {
	        	strcpy(who_adr,"GLOBAL");
	            //printf("%s global bir MAC adresidir.\n", mac_str);
	        }
	        printf("%s %s bir MAC adresidir.\n", mac_str, who_adr);
	    } else {
	        printf("%s geçersiz bir MAC adresidir.\n", mac_str);
	    }
}
uint8_t is_local_mac(const char *mac) {
    // İlk byte'ın ikinci biti 1 ise yerel MAC adresidir
    unsigned char first_byte;
    sscanf(mac, "%hhx", &first_byte);
    return (first_byte & 0x02) != 0;
}
void generate_unique_mac(void) {
	if (custom_file_mac == 0) {
		uint32_t uid0 = *(uint32_t*)UID_BASE;      // UID'nin ilk 32 biti
		uint32_t uid1 = *(uint32_t*)(UID_BASE + 4); // UID'nin sonraki 32 biti
		uint32_t uid2 = *(uint32_t*)(UID_BASE + 8); // UID'nin son 32 biti
		// Tüm MAC adresini sıfırla
		memset(custom_mac_address, 0, MAC_ADDRESS_LENGTH);
		// MAC adresinin ilk baytı: Yerel yönetimli MAC adresi (06:xx:xx:xx:xx:xx)
		custom_mac_address[0] = 0x06; // Yerel yönetimli MAC adresi (Unicast ve Locally Administered)
		// UID'den MAC adresinin geri kalanını türet
		custom_mac_address[1] = (uid0 >> 16) & 0xFF;
		custom_mac_address[2] = uid0 & 0xFF;
		custom_mac_address[3] = (uid1 >> 8) & 0xFF;
		custom_mac_address[4] = uid1 & 0xFF;
		custom_mac_address[5] = uid2 >> 24 & 0xFF;  // uid2'den bir bayt ekle
		// İsteğe bağlı: MAC adresinin benzersizliğini artırmak için ek bir rastgele sayı ekle
		// (Ancak, bu rastgele sayıyı kalıcı olarak saklamanız veya yeniden üretmeniz gerekir)
	} else {
    	custom_mac_address[0] = 0xBC;
    	custom_mac_address[1] = 0x24;
    	custom_mac_address[2] = 0x11;
    	custom_mac_address[3] = 0x2C;
    	custom_mac_address[4] = 0xF5;
    	custom_mac_address[5] = 0x42;
    	//"BC:24:11:2C:F5:42"
    }
}
//void generate_unique_1mac11(uint8_t *mac_address) {
//    // UID'nin başlangıç adresi (STM32F4 için)
//    uint32_t uid0 = *(uint32_t*)UID_BASE;      // UID'nin ilk 32 biti
//    uint32_t uid1 = *(uint32_t*)(UID_BASE + 4); // UID'nin sonraki 32 biti
//    uint32_t uid2 = *(uint32_t*)(UID_BASE + 8); // UID'nin son 32 biti
//    // MAC adresinin ilk baytı: Yerel yönetimli MAC adresi (02:xx:xx:xx:xx:xx)
//    mac_address[0] = 0x02; // Yerel yönetimli MAC adresi
//    // MAC adresinin geri kalanını UID'den türet
//    mac_address[1] = (uid0 >> 8) & 0xFF;
//    mac_address[2] = uid0 & 0xFF;
//    mac_address[3] = (uid1 >> 24) & 0xFF;
//    mac_address[4] = (uid1 >> 16) & 0xFF;
//    mac_address[5] = (uid1 >> 8) & 0xFF;
//}
// Mikrodenetleyicinin UID değerini okuyan fonksiyon
void get_uid_string(char *uid_str) {
    uint32_t uid_part1 = READ_REG(*((uint32_t*) (UID_BASE)));
    uint32_t uid_part2 = READ_REG(*((uint32_t*) (UID_BASE + 4U)));
    uint32_t uid_part3 = READ_REG(*((uint32_t*) (UID_BASE + 8U)));
    sprintf(uid_str, "%08X%08X%08X", (unsigned int)uid_part1, (unsigned int)uid_part2, (unsigned int)uid_part3);
}
void generate_barcode(char *barcode_str, uint8_t company_id, char *project_id, char *function_id, char *erelase, uint8_t hw_sw) {
	char uid_str[25];  // 24 karakter + '\0' null terminator
	char ucompany[3] = {'0'};
    char uproject[6] = {'0'};
    char ufunction[4] = {'0'};
    char uerelase[3] = {'0'};
    char uhw_sw[2] = {'0'};
    // company_id'nin son iki basamağını almak
	snprintf(ucompany, sizeof(ucompany), "%02d", company_id %100);
	 char hw_sw_str[4];
	snprintf(hw_sw_str, sizeof(hw_sw_str), "%03d", hw_sw);
	uhw_sw[0] = hw_sw_str[strlen(hw_sw_str) - 1];  // son karakteri al
	uhw_sw[1] = '\0';  // null terminator ekle
	// hw_sw'nin son iki basamağını almak
    strncpy(uproject, project_id, sizeof(uproject) - 1);
    strncpy(ufunction, function_id, sizeof(ufunction) - 1);
    strncpy(uerelase, erelase, sizeof(uerelase) - 1);
	get_uid_string(uid_str);
	if (hw_sw == 1) //                                02        05        03        26        02         01
		snprintf(barcode_str, 40, "*%s%s%s%s%s%s*", ucompany, uproject, ufunction, uid_str, uerelase, uhw_sw);
	else
		snprintf(barcode_str, 19, "*%s%s%s%s%s*", ucompany, uproject, ufunction, uerelase, uhw_sw);
}
/**
 * @brief printf için Mikroislemciden reform_hardware_code fonksiyonu
 * @param ptr Yazılacak veri
 * @param len Veri uzunluğu
 * @return Yazılan byte sayısı
 */
void generate_unique_barcode(char *barcode_str, uint16_t company_id, char *project_id, char *function_id, char *erelase) {
	generate_barcode(barcode_str, company_id, project_id, function_id, erelase, 0);
}
/**
 * @brief printf için Mikroislemciden reform_hardware_code fonksiyonu
 * @param ptr Yazılacak veri
 * @param len Veri uzunluğu
 * @return Yazılan byte sayısı
 */
void reform_hardware_code(char *barcode_str, uint16_t hw_companyid, char *hw_projectid, char *hw_functionid, char *hw_erelase) {
	generate_barcode(barcode_str, hw_companyid, hw_projectid, hw_functionid, hw_erelase, 1);
}
/**
 * @brief printf için Mikroislemciden reform_software_code fonksiyonu
 * @param ptr Yazılacak veri
 * @param len Veri uzunluğu
 * @return Yazılan byte sayısı
 */
void reform_software_code(char *barcode_str, uint16_t sw_companyid, char *sw_projectid, char *sw_functionid, char *sw_erelase) {
	generate_barcode(barcode_str, sw_companyid, sw_projectid, sw_functionid, sw_erelase, 2);
}
/*
 * uint8_t mac_address[6]; // MAC adresini tutacak dizi
generate_unique_mac(mac_address); // MAC adresini oluştur

// MAC adresini ekrana yazdır
printf("Generated MAC Address: %02X:%02X:%02X:%02X:%02X:%02X\n",
       mac_address[0], mac_address[1], mac_address[2],
       mac_address[3], mac_address[4], mac_address[5]);
 */
 /**
 * @brief verilen string fonksionundan bir indexinden veri döndürür 
 * @param index kaçıncı sıradaki değeri
 * @param delimiter stringin ayraç karekteri
 * @return dönen string
 */
char* str_parser(char *xstring, char const *delimiter, int index) {
	char string[512]; // = "Hello! We are learning about strtok";
	strcpy(string, xstring);
	char *token = strtok(string, delimiter);
	int say = 0;
	while (token != NULL) {
		//printf( " %s\n", token ); //printing each token
		if (index == say) {
			//printf( "return %s\n", token );
			memset(token, '\0', strlen(token));
			char *copy = (char*) malloc(strlen(token) + 1);
			strcpy(copy, token);
			//memset(copy, '\0', strlen(copy));
			free(token);
			return copy;
		}
		token = strtok(NULL, delimiter);
		say += 1;
	}
	free(token);
	return NULL;
}
float Celsius2Fahrenheit(float invalue) {
	return invalue * 9 / 5 + 32;
}
float Fahrenheit2Celsius(float invalue) {
	return (invalue - 32) * (5 / 9);
}
float ByteToFloat(uint8_t f_byte, uint8_t d_byte) {
	// 0=Ondalık kısmı normalize et (0-255 -> 0.0-1.0 aralığına çevrilir)
	float decimal_part = d_byte / 255.0f;
	return (float) f_byte + decimal_part;
}
uint8_t FloatToByte(float decimal_part) {
	// Ondalık kısmı (0.0-1.0 aralığı) 0-255 aralığına çevir
	return (uint8_t) (decimal_part * 255.0f);
}
void estrcpy(char* dest, const char* src, size_t dest_size) {
	//size_t dest_size = sizeof(dest) / sizeof(dest[0]);
	memset(dest, 0, dest_size); // dest dizisini sıfırlıyoruz
    size_t src_len = strlen(src);
    strcpy(dest, src);
    dest[src_len] = '\0'; // Dizeyi sonlandırıyoruz
}

