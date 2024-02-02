/*
 * Copyright (C) 2023 Orastron Srl unipersonale
 */

#include "memset.h"

void *memset(void *ptr, int value, size_t num) {
	unsigned char *p = (unsigned char *)ptr;
	for (size_t i = 0; i < num; i++)
		p[i] = (unsigned char)value;
	return ptr;
}
