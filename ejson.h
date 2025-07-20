/*
 * efunctions.h
 *
 *  Created on: Feb 10, 2025
 *      Author: mehme
 */

#ifndef ELIB_EFUNCTIONS_H_
#define ELIB_EFUNCTIONS_H_

#include "stm32f3xx.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct {
    char *key;
    char *value;
} eDictionaryEntry;

typedef struct {
	eDictionaryEntry *entries;
    size_t size;
    size_t capacity;
} eDictionary;


// Dictionary başlatma
void dict_init(eDictionary *dict);
// Dictionary sıfırlama (temizleme)
void dict_reset(eDictionary *dict);
// Bellek büyütme fonksiyonu
void dict_expand(eDictionary *dict);
// Dictionary'ye key-value ekleme
void dict_add(eDictionary *dict, const char *key, const char *value);
// Key ile değeri alma
const char *dict_get(eDictionary *dict, const char *key);
// Dictionary içeriğini yazdırma
void dict_print(const eDictionary *dict);
// Dictionary içeriğini string olarak döndüren fonksiyon
char* dict_exportstr(eDictionary *dict);
// Sayıları string'e dönüştürüp dictionary'ye ekleme
void dict_add_str(eDictionary *dict, const char *key, const char *value);
// int türü için dict_add fonksiyonu
void dict_add_int(eDictionary *dict, const char *key, int value);
// long türü için dict_add fonksiyonu
void dict_add_long(eDictionary *dict, const char *key, long value);
// float türü için dict_add fonksiyonu
void dict_add_float(eDictionary *dict, const char *key, float value);
// Belirli bir index'e yeni bir Dictionary ekleme
void dict_insert_at_index(eDictionary *dict, size_t index, eDictionary *new_dict);
void free_edictionary(eDictionary *dict);
eDictionary *loads_str(const char *str);

#endif /* ELIB_EFUNCTIONS_H_ */
