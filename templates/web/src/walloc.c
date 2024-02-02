/*
 * Copyright (C) 2021, 2022, 2024 Orastron Srl unipersonale
 */

#include "walloc.h"

#include <stdint.h>

extern unsigned char __heap_base;

typedef struct _header {
	struct _header *next;
	struct _header *prev;
	char free;
} header;

static char inited = 0;

static size_t get_size(header *h) {
	char *n = (char *)h->next;
	return (n ? n : (char *)(__builtin_wasm_memory_size(0) << 16)) - (char *)h - sizeof(header);
}

static void split_if_possible(header *h, size_t s, size_t size) {
	if (s <= size + sizeof(header) + sizeof(header))
		return;

	header *hn = (header *)((char *)h + sizeof(header) + size);
	hn->prev = h;
	hn->next = h->next;
	hn->free = 1;
	h->next = hn;
	if (hn->next)
		hn->next->prev = hn;
}

void *malloc(size_t size) {
	if (size == 0)
		return NULL;

	header *h = (header *)&__heap_base;

	if (!inited) {
		h->next = NULL;
		h->prev = NULL;
		h->free = 1;
		inited = 1;
	}

	header *p;
	for (; h; p = h, h = h->next) {
		if (!h->free)
			continue;

		size_t s = get_size(h);
		if (s < size)
			continue;
		
		split_if_possible(h, s, size);

		h->free = 0;
		return (char *)h + sizeof(header);
	}

	int32_t n = __builtin_wasm_memory_grow(0, ((size + sizeof(header) - 1) >> 16) + 1);
	if (n < 0)
		return NULL;

	if (p->free)
		h = p;
	else {
		h = (header *)(n << 16);
		p->next = h;
		h->prev = p;
		h->next = NULL;
	}

	split_if_possible(h, get_size(h), size);

	h->free = 0;
	return (char *)h + sizeof(header);
}

void *realloc(void *ptr, size_t size) {
	if (ptr == NULL)
		return malloc(size);

	if (size == 0) {
		free(ptr);
		return NULL;
	}
	
	header *h = (header *)((char *)ptr - sizeof(header));
	size_t s = get_size(h);
	if (s >= size)
		return ptr;

	void *p = malloc(size);
	if (p == NULL)
		return NULL;

	char *src = (char *)ptr;
	char *dest = (char *)p;
	for (size_t i = 0; i < s; i++)
		dest[i] = src[i];

	free(ptr);

	return p;
}

void free(void *ptr) {
	header *h = (header *)((char *)ptr - sizeof(header));
	h->free = 1;

	if (h->next && h->next->free) {
		h->next = h->next->next;
		if (h->next)
			h->next->prev = h;
	}

	if (h->prev && h->prev->free) {
		h->prev->next = h->next;
		if (h->next)
			h->next->prev = h->prev;
	}
}
