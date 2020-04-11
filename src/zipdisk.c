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
#include "rle.h"

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
        int rle_result;

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
                printf("packbyte: $%02x, data: $%04x bytes\n",
                        p[3], p[2]);

                rle_result = zcc_rle_decode(NULL, p + 4, p[3], p[2]);
                printf("rle result = %d", rle_result);

                p += p[2] + 2;
                break;
            default:
                exit(1);
        }
        p += 2;
        putchar('\n');
    }
}


static bool zcc_unpack_block(uint8_t *dest, uint8_t *src)
{
    int track = src[ZCC_ZIPDISK_TRACK] & 0x3f;
    int sector = src[ZCC_ZIPDISK_SECTOR];
    int method = src[ZCC_ZIPDISK_TRACK] >> 6U;

    printf("track %d, sector %d, pack method %d (%s)\n",
            track, sector, method, zipdisk_pack_methods[method]);

    switch (method) {
        case ZCC_PACK_NONE:
            memcpy(dest, src + ZCC_ZIPDISK_DATA, 256);
            break;
        case ZCC_PACK_FILL:
            memset(dest, src[ZCC_ZIPDISK_DATA], 256);
            break;
        case ZCC_PACK_RLE:
            if (zcc_rle_decode(dest,
                               src + ZCC_ZIPDISK_RLE_DATA,
                               src[ZCC_ZIPDISK_RLE_PACKBYTE],
                               src[ZCC_ZIPDISK_RLE_LENGTH]) != 256) {
                return false;
            }
            break;
        default:
            return false;
    }
    return true;
}

#if 0
bool zcc_zipdisk_write(zcc_zipdisk_t *zip, zcc_d64_t *d64)
{
    return true;
}
#endif


static bool iter_current_block_info(zcc_zipdisk_iter_t *iter)
{
    zcc_zipdisk_slice_t slice;
    uint8_t *data;

    slice = iter->zip->slices[iter->slice_index];

    if (iter->slice_offset >= slice.size) {
        /* end of slice, not an error per se. */
        return false;
    }

    data = slice.data + iter->slice_offset;
    iter->track = data[ZCC_ZIPDISK_TRACK] & 0x3f;
    iter->sector = data[ZCC_ZIPDISK_SECTOR];
    iter->method = data[ZCC_ZIPDISK_TRACK] >> 6U;
    iter->block_data = data;
    return true;
}


/** \brief  Dump information of the state of \a iter on stdout
 *
 * \param[in]   iter    zipdisk iterator
 */
void zcc_zipdisk_iter_dump(const zcc_zipdisk_iter_t *iter)
{
    printf("slice #%d (offset: $%04lx), blk %3d (%2d,%2d), %s\n",
            iter->slice_index, (unsigned long)(iter->slice_offset),
            iter->block_nr, iter->track, iter->sector,
            zipdisk_pack_methods[iter->method]);
}


/** \brief  Initialize zipdisk iterator \a iter with zipdisk \a zip
 *
 * \param[out]  iter    zipdisk iterator
 * \param[in]   zip     zipdisk handle
 *
 * \return  true on success
 */
bool zcc_zipdisk_iter_init(zcc_zipdisk_iter_t *iter, zcc_zipdisk_t *zip)
{
    iter->zip = zip;
    iter->slice_index = 0;
    iter->slice_offset = 4;
    iter->block_nr = 0;
    iter->block_data = NULL;

    return iter_current_block_info(iter);
}


/** \brief  Move zipdisk iterator \a iter to the next block
 *
 * \param[in,out]   iter    zipdisk iterator
 *
 * \return  true when a next block was found, false on end of archive, or error
 *
 * \throw   ZCC_ERR_ZC_INVALID_PACK_METHOD
 */
bool zcc_zipdisk_iter_next(zcc_zipdisk_iter_t *iter)
{
    int offset;         /* offset to next block in the current slice */
    uint8_t *data;      /* current block's data, includes the zipcode
                           track+packmethod,sector data */

    if (iter_current_block_info(iter)) {
        /* got current block, get the next one */
        switch (iter->method) {
            case ZCC_PACK_NONE:
                /* full raw block of 256 bytes */
                offset = 256 + 2;
                break;
            case ZCC_PACK_FILL:
                offset = 3;
                break;
            case ZCC_PACK_RLE:
                data = iter->block_data;
                offset = data[ZCC_ZIPDISK_RLE_LENGTH] + 4;
                break;
            default:
                offset = -1;
                zcc_errno = ZCC_ERR_ZC_INVALID_PACK_METHOD;
                return false;
        }
        iter->slice_offset += (size_t)offset;
        iter_current_block_info(iter);
    } else {
        /* end of slice, check if we have another one */
        printf("Getting next slice\n");
        if (iter->slice_index >= iter->zip->slice_count + 1) {
            /* end of archive */
            printf("End of archive\n");
            return false;
        }
        iter->slice_index++;
        iter->slice_offset = 2;     /* skip load address */
        if (!iter_current_block_info(iter)) {
            /* error code already set */
            return false;
        }
    }

    iter->block_nr++;
    return true;
}


/** \brief  Test hook: test the zipdisk iterator
 *
 * \param[in]   path    path to zipdisk archive
 *
 * \return  true on success
 */
bool zcc_zipdisk_test_iter(const char *path)
{
    zcc_zipdisk_t zip;
    zcc_zipdisk_iter_t iter;

    printf("Attempting to load zipdisk archive '%s' .. ", path);
    zcc_zipdisk_init(&zip);
    if (!zcc_zipdisk_read(&zip, path)) {
        printf("failed: ");
        zcc_perror(NULL);
        return false;
    }
    printf("OK.\n");

    printf("Initializing iterator .. ");
    if (!zcc_zipdisk_iter_init(&iter, &zip)) {
        printf("failed: ");
        zcc_perror(NULL);
        zcc_zipdisk_free(&zip);
        return false;
    }
    printf("OK.\n");
    zcc_zipdisk_iter_dump(&iter);

    /* iterate */
    while (zcc_zipdisk_iter_next(&iter)) {
        zcc_zipdisk_iter_dump(&iter);
    }


    printf("Cleaning up\n");
    zcc_zipdisk_free(&zip);
    return true;
}



bool zcc_zipdisk_unzip(zcc_zipdisk_t *zip, const char *path)
{
    zcc_d64_t d64;
    zcc_d64_type_t type = ZCC_D64_TYPE_CBMDOS;
    zcc_zipdisk_iter_t iter;
    uint8_t buffer[256];

    if (zip->slice_count == 5) {
        printf("Need 40-track image\n");
        /* type doesn't really matter, as long it's 40 tracks, BAM gets
         * overwritten anyway.
         */
        type = ZCC_D64_TYPE_SPEEDDOS;
    }

    /* create target D64 and allocate space and copy path */
    zcc_d64_init(&d64);
    zcc_d64_alloc(&d64, type);
    d64.path = zcc_strdup(path);

    /* init zipdisk iter */
    if (!zcc_zipdisk_iter_init(&iter, zip)) {
        zcc_d64_free(&d64);
        return false;
    }


    do {
        if (!zcc_unpack_block(buffer, iter.block_data)) {
            zcc_perror(NULL);
            zcc_d64_free(&d64);
            return false;
        }
        printf("(%2d,%2d):\n", iter.track, iter.sector);
        zcc_hexdump(buffer, 256, 0);

        zcc_d64_block_write(&d64, buffer, iter.track, iter.sector);


    } while (zcc_zipdisk_iter_next(&iter));

    zcc_d64_write(&d64, path);
    zcc_d64_free(&d64);

    return true;
}

