/*
 * Copyright (C) 2021, 2022, 2024 Orastron Srl unipersonale
 */

#ifndef WALLOC_H
#define WALLOC_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

void *malloc(size_t size);
void *realloc(void *ptr, size_t size);
void free(void *ptr);

#ifdef __cplusplus
}
#endif

#endif
