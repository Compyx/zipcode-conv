/* vim: set et ts=4 sw=4 sts=4 fdm=marker syntax=c.doxygen: */

/** \file   mem.c
 * \brief   Memory handling
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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <ctype.h>

#include "mem.h"


/** \brief  Allocate \a n bytes on the heap
 *
 * \param[in]   n   number of bytes to allocate
 *
 * \return  pointer to allocated memory
 *
 * \note    This behaves like xmalloc: if the allocation request fails, exit is
 *          called.
 */
void *zcc_malloc(size_t n)
{
    void *p = malloc(n);
    if (p == NULL) {
        fprintf(stderr, "fatal: failed to allocate %lu bytes.\n",
                (unsigned long)n);
        exit(1);
    }
    return p;
}


/** \brief  Rellocate memory at \a p to \n bytes
 *
 * \param[in,out]   p   memory to reallocate
 * \param[in]       n   new size
 *
 * \return  pointer to reallocated memory
 * \note    the pointer returned can differ from the pointer passed
 */
void *zcc_realloc(void *p, size_t n)
{
    void *tmp = realloc(p, n);
    if (tmp == NULL) {
         fprintf(stderr, "fatal: failed to allocate %lu bytes.\n",
                (unsigned long)n);
         exit(1);
    }
    return tmp;
}


/** \brief  Free memory allocated at \a p
 *
 * \param[in,out]   p   pointer to memory to free
 */
void zcc_free(void *p)
{
    free(p);
}


/** \brief  Create heap-allocated copy of string \a s
 *
 * \param[in]   s   nul-terminated string
 *
 * \return  heap-allocated copy of \a s
 */
char *zcc_strdup(const char *s)
{
    size_t len;
    char *t;

    len = strlen(s);
    t = zcc_malloc(len + 1);
    memcpy(t, s, len + 1);
    return t;
}


/** \brief  Create a hexdump of \a len bytes of \a src on stdout
 *
 * \param[in]   src     data to display
 * \param[in]   len     number of bytes to display
 * \param[in]   voffset virtual offset (displayed as the 'address')
 */
void zcc_hexdump(const uint8_t *src, size_t len, size_t voffset)
{
    uint8_t display[16];
    size_t i = 0;

    if (src == NULL || len == 0) {
        fprintf(stderr, "%s:%s:%d: error: no input\n",
                __FILE__, __func__, __LINE__);
        return;
    }

    while (i < len) {
        size_t x;
        size_t t;
        size_t c;

        printf("%05lx  ", (unsigned long)voffset);
        fflush(stdout);
        for (x = 0; x < 16 && i + x < len; x++) {
            display[x] = src[i + x];
            printf("%02x ", src[i + x]);
        }
        c = x;
        t = x;
        while (t++ < 16) {
            printf("   ");
        }
        for (t = 0; t < c; t++) {
            putchar(isprint(display[t]) ? display[t] : '.');
        }


        if (i + x >= len) {
            putchar('\n');
            return;
        }
        putchar('\n');
        i += 16;
        voffset += 16;
    }
}



