/* vim: set et ts=4 sw=4 sts=4 fdm=marker syntax=c.doxygen: */

/** \file   zipdisk.c
 * \brief   zipdisk handling
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
#include "io.h"

#include "zipdisk.h"


/** \brief  String representations of the compression methods
 */
static const char *zipdisk_pack_methods[] = {
    "store",
    "fill",
    "RLE",
    "reserved"
};


/** \brief  Initialize \a zip for use
 *
 * Initializes \a zip to a usable state.
 *
 * \param[out]  zip     zipdisk handle
 */
void zcc_zipdisk_init(zcc_zipdisk_t *zip)
{
    zip->path = NULL;
    for (int i = 0; i < ZCC_ZIPCODE_SLICE_MAX; i++) {
        zip->slices[i].data = NULL;
        zip->slices[i].size = 0;
    }
    zip->slice_count = 0;
}


/** \brief  Clean up all memory used by the member of \a zip
 *
 * \param[in,out]   zip     zipdisk handle
 */
void zcc_zipdisk_free(zcc_zipdisk_t *zip)
{
    if (zip->path != NULL) {
        zcc_free(zip->path);
    }
    for (int i = 0; i < ZCC_ZIPCODE_SLICE_MAX; i++) {
        if (zip->slices[i].data != NULL) {
            zcc_free(zip->slices[i].data);
        }
    }
}


/** \brief  Read data from \a path into \a zip
 *
 * \param[in,out]   zip     zipdisk handle
 * \param[in]       path    path to a file of the zipcoded disk image
 *
 * \return  boolean
 * \throw   ZCC_ERR_INVALID_FILENAME
 * \throw   ZCC_ERR_IO
 */
bool zcc_zipdisk_read(zcc_zipdisk_t *zip, const char *path)
{
    char *basename;
    int i;

    zip->path = zcc_strdup(path);
    basename = zcc_basename(zip->path);

    /* check basename for "[1-5]!*" */
    if (basename[1] != '!' || (basename[0] < '1' || basename[0] > '5')) {
        zcc_errno = ZCC_ERR_INVALID_FILENAME;
        zcc_free(zip->path);
        return false;
    }

    zip->slice_index = basename;

    for (i = 0; i < ZCC_ZIPCODE_SLICE_MAX - 1; i++) {
        long result;

        *(zip->slice_index) = (char)(i + 1 + '0');
        printf("reading '%s' ... ", zip->path);
        result = zcc_fread_alloc(&(zip->slices[i].data), zip->path);
        printf("%ld\n", result);

        if (result < 0) {
            if (i < ZCC_ZIPCODE_SLICE_MAX - 2) {
                zcc_errno = ZCC_ERR_IO;
                return false;
            } else {
                printf("No fifth slice found, continuing\n");
                zip->slice_count = 4;
                return true;
            }
        } else {
            zip->slices[i].size = (size_t)result;
        }

    }
    zip->slice_count = 5;

    return true;
}


/** \brief  Debug hook: dump information on \a slice in \a zip
 *
 * \param[in]   zip     zipdisk handle
 * \param[in]   slice   slice index, starting at 0 ('1!foo'_
 */
void zcc_zipdisk_dump_slice(zcc_zipdisk_t *zip, int slice)
{
    uint8_t *data = zip->slices[slice].data;
    size_t   size = zip->slices[slice].size;
    uint16_t load = (uint16_t)(data[0] + data[1] * 256);
    uint8_t *p;

    if (load == 0x3fe) {
        /* got a disk ID */
        printf("disk id = '%c%c'\n", data[2], data[3]);
        p = data + 4;
    } else {
        p = data + 2;
    }
    printf("slice #%d: load = $%04x, size = $%04x\n",
            slice, load, (unsigned int)size);


    while (p < data + size) {
        int track = p[0];
        int sector = p[1];

        printf("(%2d,%2d): %s: ",
                track & 0x3f, sector, zipdisk_pack_methods[track >> 6]);
        fflush(stdout);

        switch (track >> 6) {
            case ZCC_PACK_NONE:
                printf("raw sector data");
                p += 256;
                break;
            case ZCC_PACK_FILL:
                printf("$%02x", p[2]);
                p += 1;
                break;
            case ZCC_PACK_RLE:
                printf("packbyte: $%02x, data: $%04x bytes",
                        p[3], p[2]);
                p += p[2] + 2;
                break;
            default:
                exit(1);
        }
        p += 2;
        putchar('\n');
    }
}
