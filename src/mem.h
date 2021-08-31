/** \file   mem.h
 * \brief   Memory handling - header
 */

/*
 * This file is part of zipcode-conv
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 *  02111-1307  USA.
 *
 */

#ifndef ZCC_MEM_H
#define ZCC_MEM_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

void *zcc_malloc(size_t n);
void *zcc_calloc(size_t nmemb, size_t size);
void *zcc_realloc(void *p, size_t n);
void zcc_free(void *p);
char *zcc_strdup(const char *s);
void zcc_hexdump(const uint8_t *src, size_t len, size_t voffset);
int zcc_popcount_byte(uint8_t b);

#endif
