/* vim: set et ts=4 sw=4 sts=4 fdm=marker syntax=c.doxygen: */

/** \file   rle.c
 * \brief   Run-Length Encoding handling
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

#include "errors.h"
#include "mem.h"

#include "rle.h"



/** \brief  Decode run-length encoded data
 *
 * \param[out]  dest    destination of RLE data (use `NULL` to not
 *                      copy data, useful for debugging)
 * \param[in]   src     RLE data
 * \param[in]   run     RLE 'run' byte
 * \param[in]   len     number of bytes to decode of \a src
 *
 * \return  size of decode data (should be 256)
 * \throw   ZCC_ERR_RLE
 */
int zcc_rle_decode(uint8_t *dest, const uint8_t *src, int run, int len)
{
    uint8_t buffer[256];
    int s = 0;  /* source index */
    int b = 0;  /* buffer index */

    memset(buffer, 0, sizeof(buffer));

    while (s < len && b < 256) {
        if (src[s] == run) {
            /* run */
            int r = src[s + 1];
            while (--r > 0 && b < 256) {
                buffer[b++] = src[s + 2];
            }
            s += 2;
        } else {
            buffer[b++] = src[s++];
        }
    }

    if (b == 256) {
        if (dest != NULL) {
            memcpy(dest, buffer, 256);
        }
    } else {
        zcc_errno = ZCC_ERR_RLE;
    }

    /* debug */
    zcc_hexdump(buffer, 256, 0);

    return b;
}
