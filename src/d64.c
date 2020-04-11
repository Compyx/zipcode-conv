/* vim: set et ts=4 sw=4 sts=4 fdm=marker syntax=c.doxygen: */

/** \file   d64.c
 * \brief   D64 handling
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

#include "d64.h"


/** \brief  DOS type strings
 */
static const char *dos_types[] = {
    "CBM DOS",
    "SpeedDOS",
    "DolphinDOS",
    "Professional DOS",
    "Prologic DOS"
};


/** \brief  Speed zones table for D64 images
 */
static const zcc_d64_speedzone_t speedzones[] = {
    {  1, 17, 21 },
    { 18, 24, 19 },
    { 25, 30, 18 },
    { 31, 40, 17 }
};




/** \brief  Get offset in bytes for block at (\a track, \a sector)
 *
 * \param[in]   track   track number
 * \param[in]   sector  sector number
 *
 * \return  offset in bytes or -1 on failure
 * \throw   ZCC_ERR_TRACK_RANGE
 * \throw   ZCC_ERR_SECTOR_RANGE
 */
long zcc_d64_block_offset(int track, int sector)
{
    int zone = 0;
    long offset = 0;
    int tracks;

    /* preliminary checks on track and sector number */
    if (track < ZCC_D64_TRACK_MIN || track > ZCC_D64_TRACK_MAX_EXT) {
        zcc_errno = ZCC_ERR_TRACK_RANGE;
        return -1;
    }
    if (sector < ZCC_D64_SECTOR_MIN || sector > ZCC_D64_SECTOR_MAX) {
        zcc_errno = ZCC_ERR_SECTOR_RANGE;
        return -1;
    }

    while (zone < (int)(sizeof speedzones / sizeof speedzones[0])
            && (track > speedzones[zone].track_max)) {
        /* add complete zone */
        tracks = speedzones[zone].track_max - speedzones[zone].track_min + 1;
        offset += tracks * ZCC_D64_BLOCK_SIZE_RAW * speedzones[zone].sectors;
        zone++;
    }

    /* final sector number check */
    if (sector >= speedzones[zone].sectors) {
        return -1;
    }

    tracks = track - speedzones[zone].track_min;
    offset += tracks * ZCC_D64_BLOCK_SIZE_RAW * speedzones[zone].sectors;
    return offset + sector * ZCC_D64_BLOCK_SIZE_RAW;
}


/** \brief  Get offset in bytes for \a track
 *
 * \param[in]   track   track number
 *
 * \return  offset in bytes or -1 on failure
 * \throw   ZCC_ERR_TRACK_RANGE
 */
long zcc_d64_track_offset(int track)
{
    return zcc_d64_block_offset(track, 0);
}


/** \brief  Check if \a track number is valid for \a d64
 *
 * \param[in]   d64     D64 handle
 * \param[in]   track   track number
 *
 * \return  true if \a track is valid for image \a d64
 * \throw   ZCC_ERR_TRACK_RANGE
 */
bool zcc_d64_track_is_valid(const zcc_d64_t *d64, int track)
{
    /* check track number if 35-track image */
    if (d64->type == ZCC_D64_TYPE_CBMDOS) {
        if (track < ZCC_D64_TRACK_MIN || track > ZCC_D64_TRACK_MAX) {
            zcc_errno = ZCC_ERR_TRACK_RANGE;
            return false;
        }
    }
    if (track < ZCC_D64_TRACK_MIN || track > ZCC_D64_TRACK_MAX_EXT) {
        zcc_errno = ZCC_ERR_TRACK_RANGE;
        return false;
    }
    return true;
}


/** \brief  Read block (\a track,\a sector) in \a d64 into \a buffer
 *
 * \param[in]   d64     D64 handle
 * \param[out]  buffer  buffer to store block data
 * \param[in]   track   track number of block
 * \param[in]   sector  sector number of sector
 *
 * \return  true on success
 * \throw   ZCC_ERR_TRACK_RANGE
 * \throw   ZCC_ERR_SECTOR_RANGE
 */
bool zcc_d64_block_read(const zcc_d64_t *d64,
                        uint8_t *buffer,
                        int track, int sector)
{
    long offset;

    if (!zcc_d64_track_is_valid(d64, track)) {
        return false;
    }

    offset = zcc_d64_block_offset(track, sector);
    if (offset < 0) {
        return false;
    }

    memcpy(buffer, d64->data + offset, ZCC_D64_BLOCK_SIZE_RAW);
    return true;
}


/** \brief  Write \a buffer into \a d64 at block (\a track,\a sector)
 *
 * \param[in]   d64     D64 handle
 * \param[out]  buffer  buffer to read data from
 * \param[in]   track   track number of block
 * \param[in]   sector  sector number of sector
 *
 * \return  true on success
 * \throw   ZCC_ERR_TRACK_RANGE
 * \throw   ZCC_ERR_SECTOR_RANGE
 */
bool zcc_d64_block_write(zcc_d64_t *d64,
                         const uint8_t *buffer,
                         int track, int sector)
{
    long offset;

    if (!zcc_d64_track_is_valid(d64, track)) {
        return false;
    }

    offset = zcc_d64_block_offset(track, sector);
    if (offset < 0) {
        return false;
    }

    memcpy(d64->data + offset, buffer, ZCC_D64_BLOCK_SIZE_RAW);
    return true;
}




/** \brief  Initialize \a d64 for use
 *
 * \param[in,out]   d64 D64 handle
 */
void zcc_d64_init(zcc_d64_t *d64)
{
    d64->path = NULL;
    d64->data = NULL;
    d64->size = 0;
    d64->type = ZCC_D64_TYPE_CBMDOS;
}


/** \brief  Allocate memory in \a d64 for a D64 of \a type
 *
 * \param[in,out]   d64     D64 handle
 * \param[in]       type    D64 type
 */
void zcc_d64_alloc(zcc_d64_t *d64, zcc_d64_type_t type)
{
    size_t size;

    if (type ==  ZCC_D64_TYPE_CBMDOS) {
        size = ZCC_D64_SIZE_CBMDOS;
    } else {
        size = ZCC_D64_SIZE_EXTENDED;
    }

    d64->data = zcc_calloc(size, 1LU);
    d64->size = size;
}


/** \brief  Free memory used by member of \a d64
 *
 * \param[in,out]   d64 D64 handle
 */
void zcc_d64_free(zcc_d64_t *d64)
{
    if (d64->path != NULL) {
        zcc_free(d64->path);
    }
    if (d64->data != NULL) {
        zcc_free(d64->data);
    }
}


/** \brief  Read D64 file
 *
 * \param[in,out]   d64     D64 handle
 * \param[in]       path    path to D64 image file
 * \param[in]       type    D64 type (ignored when 35 tracks)
 *
 * \return  TRUE if succesfull
 */
bool zcc_d64_read(zcc_d64_t *d64, const char *path, zcc_d64_type_t type)
{
    long result;

    /* Attempt to load image data */
    result = zcc_fread_alloc(&(d64->data), path);
    printf("got %ld bytes\n", result);
    if (result != ZCC_D64_SIZE_CBMDOS && result != ZCC_D64_SIZE_EXTENDED) {
        /* Failed */
        fprintf(stderr, "error: invalid image size\n");
        zcc_free(d64->data);
        d64->data = NULL;
        return false;
    }

    /* OK */
    d64->path = zcc_strdup(path);
    d64->size = (size_t)result;
    if (result == ZCC_D64_SIZE_CBMDOS) {
        d64->type = ZCC_D64_TYPE_CBMDOS;
    } else {
        d64->type = type;
    }
    return true;
}


/** \brief  Dump some generic info about \a d64 on stdout
 *
 * \param[in]   d64     D64 handle
 */
void zcc_d64_dump_info(const zcc_d64_t *d64)
{
    printf("type: %s\n", dos_types[d64->type]);
    printf("path: %s\n", d64->path != NULL ? d64->path : "<unset>");
    printf("size: $%lx\n", (unsigned long)d64->size);
}


/** \brief  Show a hexdump of the BAM of \a d64
 *
 * \param[in]   d64 D64 handle
 */
void zcc_d64_dump_bam(const zcc_d64_t *d64)
{
    zcc_hexdump(d64->data + 0x16500, 256, 0x16500);
}



bool zcc_d64_write(zcc_d64_t *d64, const char *path)
{
    if (path == NULL && d64->path == NULL) {
        zcc_errno = ZCC_ERR_INVALID_FILENAME;
        return false;
    }

    /* use new path? */
    if (path != NULL) {
        if (d64->path != NULL) {
            zcc_free(d64->path);
            d64->path = zcc_strdup(path);
        }
    }

    if (zcc_fwrite(d64->path, d64->data, d64->size)) {
        return false;
    }
    return true;
}
