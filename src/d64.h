/* vim: set et ts=4 sw=4 sts=4 fdm=marker syntax=c.doxygen: */

/** \file   d64.h
 * \brief   D64 handling - header
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

#ifndef ZCC_D64_H
#define ZCC_D64_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>


/** \brief  Size of a standard 35-track D64 image without error info
 */
#define ZCC_D64_SIZE_CBMDOS     174848

/** \brief  Sizeo of a 40-track D64 image without error info
 */
#define ZCC_D64_SIZE_EXTENDED   (ZCC_D64_SIZE_CBMDOS + 5 * 17 * 256)


/** \brief  Minimum track number for D64 images
 */
#define ZCC_D64_TRACK_MIN       1

/** \brief  Maximum track number for standard (35-track) D64 images
 */
#define ZCC_D64_TRACK_MAX       35

/** \brief  Maximum track number for extended (40-track) D64 images
 */
#define ZCC_D64_TRACK_MAX_EXT   40

/** \brief  Minimum sector number for D64 images
 */
#define ZCC_D64_SECTOR_MIN       0

/** \brief  Maximum sector number for D64 images
 *
 * Only valid for the first speedzone, tracks 1-17, extra checks via the
 * speedzone table are required for proper checking
 */
#define ZCC_D64_SECTOR_MAX      20

/** \brief  Size of a raw block (sector)
 *
 * This includes the 2-byte pointer to the next block (track,sector)
 */
#define ZCC_D64_BLOCK_SIZE_RAW  256

/** \brief  Size of the data section of a block
 */
#define ZCC_D64_BLOCK_SIZE_DATA 254


/** \brief  D64 types
 *
 */
typedef enum zcc_d64_type_e {
    ZCC_D64_TYPE_CBMDOS,        /**< standard CBM DOS 35 tracks */
    ZCC_D64_TYPE_SPEEDDOS,      /**< SpeedDOS 40 tracks */
    ZCC_D64_TYPE_DOLPHINDOS,    /**< DolphinDOS 40 tracks */
    ZCC_D64_TYPE_PROFDOS,       /**< ProfDOS 40 tracks */
    ZCC_D64_TYPE_PROLOGICDOS    /**< ProLogic 40 tracks */
} zcc_d64_type_t;


/** \brief  D64 speedzone entry
 */
typedef struct zcc_d64_speedzone_s {
    int track_min;      /**< first track number of the zone */
    int track_max;      /**< last track number of the zone */
    int sectors;        /**< sectors per track in the zone */
} zcc_d64_speedzone_t;


/** \brief  D64 handle
 *
 */
typedef struct zcc_d64_s {
    char *          path;   /**< path to image file */
    uint8_t *       data;   /**< binary data */
    size_t          size;   /**< size of data */
    zcc_d64_type_t  type;   /**< DOS type */
} zcc_d64_t;


long zcc_d64_block_offset(int track, int sector);
long zcc_d64_track_offset(int track);
bool zcc_d64_track_is_valid(const zcc_d64_t *d64, int track);
void zcc_d64_init(zcc_d64_t *d64);
void zcc_d64_free(zcc_d64_t *d64);
bool zcc_d64_read(zcc_d64_t *d64, const char *path, zcc_d64_type_t type);
void zcc_d64_dump_info(const zcc_d64_t *d64);
void zcc_d64_dump_bam(const zcc_d64_t *d64);

bool zcc_d64_block_read(const zcc_d64_t *d64,
                        uint8_t *buffer,
                        int track, int sector);
bool zcc_d64_block_write(zcc_d64_t *d64,
                         const uint8_t *buffer,
                         int track, int sector);

#endif

