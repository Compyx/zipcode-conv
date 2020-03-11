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

#include "mem.h"


/** \brief  Allocate \a n bytes on the heap
 *
 * \param[in    n   number of bytes to allocate
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
