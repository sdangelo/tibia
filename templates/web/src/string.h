/*
 * Copyright (C) 2023 Orastron Srl unipersonale
 */

#ifndef STRING_H
#define STRING_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

void *memset(void *ptr, int value, size_t num);
void *memcpy(void *dest, const void *src, size_t num);

#ifdef __cplusplus
}
#endif

#endif
