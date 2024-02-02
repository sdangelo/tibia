/*
 * Copyright (C) 2023 Orastron Srl unipersonale
 */

#ifndef MEMSET_H
#define MEMSET_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

void *memset(void *ptr, int value, size_t num);

#ifdef __cplusplus
}
#endif

#endif
