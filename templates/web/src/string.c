/*
 * Tibia
 *
 * Copyright (C) 2023, 2024 Orastron Srl unipersonale
 *
 * Tibia is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 *
 * Tibia is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Tibia.  If not, see <http://www.gnu.org/licenses/>.
 *
 * File author: Stefano D'Angelo
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
