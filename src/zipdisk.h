/* vim: set et ts=4 sw=4 sts=4 fdm=marker syntax=c.doxygen: */

/** \file   zipdisk.h
 * \brief   zipdisk handling - header
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

#ifndef ZCC_ZIPDISK_H
#define ZCC_ZIPDISK_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "d64.h"


#define ZCC_ZIPCODE_SLICE_MAX   6

/** \brief  Offset in a zipcoded block of the track number/compression method
 *
 * Track number is the lowest 6 bits, compression is the highest 2 bits, but
 * %11 is illegal.
 */
#define ZCC_ZIPDISK_TRACK   0

/** \brief  Offset in a zipcode block of the sector number
 */
#define ZCC_ZIPDISK_SECTOR  1

/** \brief  Offset in a zipcode block of its data, depends on the compression
 *          method
 *
 * %00: store (256 bytes)
 * %01: fill (1 byte)
 * %10: rle (variable size)
 * %11: illegal
 */
#define ZCC_ZIPDISK_DATA    2

#define ZCC_ZIPDISK_RLE_LENGTH 2
#define ZCC_ZIPDISK_RLE_PACKBYTE 3
#define ZCC_ZIPDISK_RLE_DATA 4


/** \brief  Compression types for zipcoded disks
 */
typedef enum zcc_pack_e {
    ZCC_PACK_NONE,      /**< full copy of block */
    ZCC_PACK_FILL,      /**< fill block with one value */
    ZCC_PACK_RLE,       /**< run-length encoded data */
    ZCC_PACK_INVALID    /**< invalid compression method */
} zcc_pack_t;


/** \brief  Slice of a zipcoded disk
 *
 * Represents the data in a '[1-5]!*' file.
 *
 * The filename of the slice can be reconstructed via its parent #zcc_disk_t
 */
typedef struct zcc_zipdisk_slice_s {
    uint8_t *   data;   /**< file data */
    size_t      size;   /**< file size */
} zcc_zipdisk_slice_t;


/** \brief  Zipcoded disk handle
 */
typedef struct zcc_zipdisk_s {

    /** \brief  Path to files
     *
     * Updated while reading/writing via slice_index
     */
    char *path;

    /** \brief  Location in path of the slice index (ie '1'-'5')
     */
    char *slice_index;

    /** \brief  List of slices
     */
    zcc_zipdisk_slice_t slices[ZCC_ZIPCODE_SLICE_MAX];

    /** \brief  Number of slices
     */
    int slice_count;
} zcc_zipdisk_t;


typedef struct zcc_zipdisk_iter_s {
    zcc_zipdisk_t * zip;
    int             slice_index;
    size_t          slice_offset;
    int             block_nr;
    uint8_t *       block_data;
    int             track;
    int             sector;
    int             method;
} zcc_zipdisk_iter_t;


void zcc_zipdisk_init(zcc_zipdisk_t *zip);
void zcc_zipdisk_free(zcc_zipdisk_t *zip);

bool zcc_zipdisk_read(zcc_zipdisk_t *zip, const char *path);
void zcc_zipdisk_dump_slice(zcc_zipdisk_t *zip, int slice);

bool zcc_zipdisk_write(zcc_zipdisk_t *zip, zcc_d64_t *d64);

bool zcc_zipdisk_iter_init(zcc_zipdisk_iter_t *iter, zcc_zipdisk_t *zip);
bool zcc_zipdisk_iter_next(zcc_zipdisk_iter_t *iter);
void zcc_zipdisk_iter_dump(const zcc_zipdisk_iter_t *iter);


/*
 * Testing/debuggin hooks
 */
bool zcc_zipdisk_test_iter(const char *path);

#endif
