/*
 * Copyright (C) 2023, 2024 Orastron Srl unipersonale
 */

#include "string.h"

void *memset(void *ptr, int value, size_t num) {
	unsigned char *p = (unsigned char *)ptr;
	for (size_t i = 0; i < num; i++)
		p[i] = (unsigned char)value;
	return ptr;
}

void *memcpy(void *dest, const void *src, size_t num) {
	char *d = (char *)dest;
	char *s = (char *)src;
	for (size_t i = 0; i < num; i++)
		d[i] = s[i];
	return dest;
}
