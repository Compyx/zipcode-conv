/* vim: set et ts=4 sw=4 sts=4 fdm=marker syntax=c.doxygen: */

/** \file   io.c
 * \brief   I/O handling
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
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>

#include "errors.h"
#include "mem.h"

#include "io.h"


/** \brief  Block size for zcc_fread_alloc()
 */
#define FRA_BLOCK_SIZE  65536


/** \brief  Read data from \a path, allocating memory
 *
 * This function allocates and resizes its buffer while reading data.
 *
 * \param[out]  dest    location to store pointer to data
 * \param[in]   path    path to file to read data from
 *
 * \return  number of bytes read, or -1 on error
 */
long zcc_fread_alloc(uint8_t **dest, const char *path)
{
    uint8_t *buffer;
    size_t bufsize = FRA_BLOCK_SIZE;
    size_t bufread = 0;
    size_t result;
    FILE *fp;

    errno = 0;
    *dest = NULL;
    fp = fopen(path, "rb");
    if (fp == NULL) {
        zcc_errno = ZCC_ERR_IO;
        return -1;
    }

    buffer = zcc_malloc(FRA_BLOCK_SIZE);

    while (1) {
#ifdef ZCC_BASE_DEBUG
        printf("requesting %lu bytes: ", (unsigned long)FRA_BLOCK_SIZE);
        fflush(stdout);
#endif
        result = fread(buffer + bufread, 1, FRA_BLOCK_SIZE, fp);
#ifdef ZCC_BASE_DEBUG
        printf("got %lu bytes\n", (unsigned long)result);
#endif
        if (result < FRA_BLOCK_SIZE) {
            /* end of file ? */
            if (feof(fp)) {
                /* we're done */
                bufread += result;
                if (bufread == 0) {
                    /* empty file: free buffer, dest is NULL so we're safe
                     * from free()'ing dest failing */
                    free(buffer);
                } else {
                    /* realloc buffer, if it fails we still have the data: */
                    buffer = zcc_realloc(buffer, bufread);
                    *dest = buffer;
#ifdef ZCC_BASE_DEBUG
                    printf("reallocated to %lu bytes\n",
                            (unsigned long)bufsize);
#endif
                }
                fclose(fp);
                return (long)bufread;
            } else {
                /* I/O error */
                zcc_errno = ZCC_ERR_IO;
                free(buffer);
                fclose(fp);
                return -1;
            }
        } else {
            /* resize buffer */
            bufsize += FRA_BLOCK_SIZE;
            bufread += FRA_BLOCK_SIZE;
#ifdef ZCC_BASE_DEBUG
            printf("resizing buffer to %lu bytes\n", (unsigned long)bufsize);
#endif
            buffer = realloc(buffer, bufsize);
        }
    }
    return -1;
}


/** \brief  Write \a size bytes of \a data to \a path
 *
 * \param[in]   path    file to write \a data to
 * \param[in]   data    data to write to \a path
 * \param[in]   size    number of bytes of \a data to write to \a path
 *
 * \return  TRUE on success, FALSE on failure
 * \throw   ZCC_ERR_IO
 */
bool zcc_fwrite(const char *path, const uint8_t *data, size_t size)
{
    FILE *fp;
    size_t result;

    /* open file for writing */
    fp = fopen(path, "wb");
    if (fp == NULL) {
        zcc_errno = ZCC_ERR_IO;
        return false;
    }

    result = fwrite(data, 1U, size, fp);
    if (result != size) {
        zcc_errno = ZCC_ERR_IO;
        fclose(fp);
        return false;
    }

    fclose(fp);
    return true;
}
