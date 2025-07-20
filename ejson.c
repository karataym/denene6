/*
 * efunctions.c
 *
 *  Created on: Feb 10, 2025
 *      Author: mehme const char *dict_get(eDictionary *dict, const char *key)
 */

#include "ejson.h"

// int türü için dict_add fonksiyonu
void dict_add_int(eDictionary *dict, const char *key, int value) {
    char value_str[50];
    memset(value_str, 0, sizeof(value_str));
    sprintf(value_str, "%d", value); // int'i string'e dönüştürme
    dict_add_str(dict, key, value_str);
}

// long türü için dict_add fonksiyonu
void dict_add_long(eDictionary *dict, const char *key, long value) {
    char value_str[50];
    memset(value_str, 0, sizeof(value_str));
    sprintf(value_str, "%ld", value); // long'u string'e dönüştürme
    dict_add_str(dict, key, value_str);
}

// float türü için dict_add fonksiyonu
void dict_add_float(eDictionary *dict, const char *key, float value) {
    char value_str[50];
    memset(value_str, 0, sizeof(value_str));
    sprintf(value_str, "%.2f", value); // float'ı string'e dönüştürme
    dict_add_str(dict, key, value_str);
}
// Temizleme fonksiyonu
void free_edictionary(eDictionary *dict) {
    for (size_t i = 0; i < dict->size; ++i) {
        free(dict->entries[i].key);
        free(dict->entries[i].value);
    }
    free(dict->entries);
    free(dict);
}
// Stringden alıntı işaretlerini kaldırır
char *strip_quotes(char *str) {
    if (str[0] == '"' && str[strlen(str) - 1] == '"') {
        str[strlen(str) - 1] = '\0';
        return str + 1;
    }
    return str;
}
// Bir JSON stringi parçalar ve anahtar-değer çifti olarak döndürür
void parse_kv(char *str, char **key, char **value) {
    *key = strtok(str, ":");
    *value = strtok(NULL, ",");
    if (*key) *key = strip_quotes(*key);
    if (*value) *value = strip_quotes(*value);
}
// eDictionary'ye yeni bir giriş ekler
void add_entry(eDictionary *dict, const char *key, const char *value) {
    if (dict->size >= dict->capacity) {
        dict->capacity *= 2;
        dict->entries = realloc(dict->entries, dict->capacity * sizeof(eDictionaryEntry));
    }
    dict->entries[dict->size].key = strdup(key);
    dict->entries[dict->size].value = strdup(value);
    dict->size++;
}
// JSON stringi eDictionary yapısına dönüştürür
eDictionary *loads_str(const char *str) {
    eDictionary *dict = malloc(sizeof(eDictionary));
    dict->size = 0;
    dict->capacity = 16; // Başlangıç kapasitesi
    dict->entries = malloc(dict->capacity * sizeof(eDictionaryEntry));
    char *json_copy = strdup(str);
    char *token = strtok(json_copy, "{");

    if (token == NULL) {
        free(json_copy);
        free(dict->entries);
        free(dict);
        return NULL; // Geçersiz JSON
    }
    token = strtok(NULL, "}");
    if (token == NULL) {
        free(json_copy);
        free(dict->entries);
        free(dict);
        return NULL; // Geçersiz JSON
    }
    char *pair = strtok(token, "\n");
    while (pair != NULL) {
        char *key, *value;
        parse_kv(pair, &key, &value);
        if (key && value) {
            add_entry(dict, key, value);
        }
        pair = strtok(NULL, "\n");
    }
    free(json_copy);
    return dict;
}
//json.loads(s)

// Dictionary içeriğini string olarak döndüren fonksiyon
char* dict_exportstr(eDictionary *dict) {
    size_t total_length = 2; // Başlangıçta "{" ve "}" karakterleri için yer ayırıyoruz.
    for (size_t i = 0; i < dict->size; i++) {
        total_length += strlen(dict->entries[i].key) + strlen(dict->entries[i].value) + 6; // 6: ": ", "\", ", ve "\n"
    }

    char *result = (char *)malloc(total_length * sizeof(char));
    if (result == NULL) {
        return NULL; // Bellek hatası
    }

    strcpy(result, "{\n");
    for (size_t i = 0; i < dict->size; i++) {
        char temp[256]; // Geçici bir buffer
        sprintf(temp, "  \"%s\": \"%s\"", dict->entries[i].key, dict->entries[i].value);
        strcat(result, temp);
        if (i < dict->size - 1) {
            strcat(result, ",\n");
        } else {
            strcat(result, "\n");
        }
    }
    strcat(result, "}");

    return result;
}

// Belirli bir index'e yeni bir Dictionary ekleme
void dict_insert_at_index(eDictionary *dict, size_t index, eDictionary *new_dict) {
    if (index > dict->size) {
        printf("Geçersiz index!\n");
        return;
    }
    // Kapasiteyi kontrol et ve gerekirse büyüt
    if (dict->size + new_dict->size > dict->capacity) {
        dict_expand(dict);
    }
    // Mevcut elemanları sağa kaydır
    for (size_t i = dict->size; i > index; i--) {
        dict->entries[i] = dict->entries[i - 1];
    }
    // Yeni Dictionary'nin elemanlarını ekle
    for (size_t i = 0; i < new_dict->size; i++) {
        dict->entries[index + i].key = strdup(new_dict->entries[i].key);
        dict->entries[index + i].value = strdup(new_dict->entries[i].value);
    }
    // Yeni eleman sayısını güncelle
    dict->size += new_dict->size;
}


// ✅ Dictionary Başlatma (dict_init)
void dict_init(eDictionary *dict) {
    if (!dict) return;
    dict->entries = NULL;
    dict->size = 0;
    dict->capacity = 0;
}

// ✅ Dictionary Genişletme (dict_expand)
void dict_expand(eDictionary *dict) {
    if (!dict) return;

    size_t new_capacity = (dict->capacity == 0) ? 2 : dict->capacity * 2;
    eDictionaryEntry *new_entries = realloc(dict->entries, new_capacity * sizeof(eDictionaryEntry));

    if (!new_entries) {
        fprintf(stderr, "HATA: Bellek genişletme başarısız oldu!\n");
        return;
    }
    dict->entries = new_entries;
    dict->capacity = new_capacity;
}
const char *dict_get(eDictionary *dict, const char *key) {
	for (size_t i = 0; i < dict->size; i++) {
		if (strcmp(dict->entries[i].key, key) == 0) {
			return dict->entries[i].value;
		}
	}
	return NULL;
}
// ✅ Dictionary'ye Yeni Veri Ekleme veya Güncelleme (dict_add_str)
void dict_add_str(eDictionary *dict, const char *key, const char *value) {
    if (!dict || !key || !value) return;

    // Aynı key zaten varsa, güncelle
    for (size_t i = 0; i < dict->size; i++) {
        if (strcmp(dict->entries[i].key, key) == 0) {
            free(dict->entries[i].value);  // Eski değeri temizle
            dict->entries[i].value = strdup(value);  // Yeni değeri ata
            return;
        }
    }

    // Eğer kapasite dolduysa genişlet
    if (dict->size >= dict->capacity) {
        dict_expand(dict);
        if (!dict->entries) return; // Bellek tahsisi başarısız olduysa çık
    }

    // Yeni giriş ekle
    dict->entries[dict->size].key = strdup(key);
    dict->entries[dict->size].value = strdup(value);
    dict->size++;
}

// ✅ Dictionary'den Veri Silme (dict_remove)
void dict_remove(eDictionary *dict, const char *key) {
    if (!dict || !key) return;

    for (size_t i = 0; i < dict->size; i++) {
        if (strcmp(dict->entries[i].key, key) == 0) {
            free(dict->entries[i].key);
            free(dict->entries[i].value);

            // Son elemanı silinen elemanın yerine taşı
            dict->entries[i] = dict->entries[dict->size - 1];
            dict->size--;
            return;
        }
    }
}
// ✅ Dictionary'yi Sıfırlama (dict_reset) (Heap Bellek Serbest Bırakılmaz!)
void dict_reset(eDictionary *dict) {
    if (!dict) return;
    for (size_t i = 0; i < dict->size; i++) {
        free(dict->entries[i].key);
        free(dict->entries[i].value);
    }
    free(dict->entries);  // Tüm girişleri tutan dizi serbest bırakılır
    dict->entries = NULL;
    dict->size = 0;
    dict->capacity = 0;
}

// ✅ Dictionary İçeriğini Yazdırma (Debug İçin)
void dict_print(const eDictionary *dict) {
    if (!dict) return;

    printf("Dictionary (%zu/%zu):\n", dict->size, dict->capacity);
    for (size_t i = 0; i < dict->size; i++) {
        printf("  [%zu] %s = %s\n", i, dict->entries[i].key, dict->entries[i].value);
    }
}








void test(void) {
	eDictionary edict;
//	dict_init(&edict, 24);  // eDictionary dict; kullanmak icin
	dict_add_str(&edict, "hello", " hella Word");
	dict_add_str(&edict, "hello1", " hella Word1");
	dict_add_str(&edict, "hello2", " hella Word2");
	dict_add_str(&edict, "hello3", " hella Word3");
	dict_add_str(&edict, "hello4", " hella Word4");
	dict_add_str(&edict, "hello5", " hella Word5");
	dict_add_str(&edict, "hello6", " hella Word6");
	//char *str1 = dict_exportstr(&edict);
	// "{\n  \"hello\": \" hella Word\",\n  \"hello1\": \" hella Word1\",\n  \"hello2\": \" hella Word2\",\n  \"hello3\": \" hella Word3\",\n  \"hello4\": \" hella Word4\",\n  \"hello5\": \" hella Word5\",\n°"
	dict_reset(&edict);
	dict_add_str(&edict, "hello", " hella Word");
	dict_add_str(&edict, "hello1", " hella Word1");
	dict_add_str(&edict, "hello2", " hella Word2");
	dict_add_str(&edict, "hello3", " hella Word3");
	dict_add_str(&edict, "hello4", " hella Word4");
	dict_add_str(&edict, "hello5", " hella Word5");
	dict_add_str(&edict, "hello6", " hella Word6");
	//str1 = dict_exportstr(&edict);
	dict_reset(&edict);
}
/*
// Örnek kullanım
int main() {
    eDictionary dict;
    dict_init(&dict, 2);  // İlk kapasite 2

    dict_add(&dict, "device", "STM32F303");
    dict_add(&dict, "version", "1.0.0");
    dict_add(&dict, "temperature", "25.5");

    printf("Dictionary İçeriği:\n");
    dict_print(&dict);

    printf("Device: %s\n", dict_get(&dict, "device"));

    // Belleği temizle
    dict_reset(&dict);

    return 0;
 tiny-json
✔ STM32 için optimize edilmiş JSON işleyici
✔ cJSON gibi çalışır ama daha hafif
👉 Link: https://github.com/rafagafe/tiny-json
*/
