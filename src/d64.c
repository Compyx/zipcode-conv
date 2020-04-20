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

#include "cbmdos.h"
#include "debug.h"
#include "errors.h"
#include "mem.h"
#include "io.h"
#include "petasc.h"

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
 * \param[out]  d64 D64 handle
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
    zcc_debug("got %ld bytes\n", result);
    if (result != ZCC_D64_SIZE_CBMDOS && result != ZCC_D64_SIZE_EXTENDED) {
        /* Failed */
        zcc_debug("error: invalid image size\n");
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


/** \brief  Write \a d64 to host file system
 *
 * Write the image in \a d64 to the host file system. If \a path is `NULL`, use
 * the path in \a d64. If that is also `NULL`, fail. Using a non-NULL \a path
 * will replace the old path in \a d64.
 *
 * \param[in,out]   d64     D64 handle
 * \param[in]       path    path to write to (NULL to use \a d64's path
 *
 * \return  boolean
 * \throw   ZCC_ERR_INVALID_FILENAME
 */
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


/** \brief  Read BAM entry for \a track in \a d64 into \a bament
 *
 * Only works for 35-track images at the moment.
 *
 * \param[in]   d64     D64 image
 * \param[out]  bament  storage for raw BAM entry (4 bytes)
 * \param[in]   track   track number
 *
 * \return  TRUE on success
 */
bool zcc_d64_bament_read(const zcc_d64_t *d64, uint8_t *bament, int track)
{
    uint8_t *bam;
    long offset;

    if (track < ZCC_D64_TRACK_MIN || track > ZCC_D64_TRACK_MAX) {
        return false;
    }

    offset = zcc_d64_block_offset(ZCC_D64_BAM_TRACK, ZCC_D64_BAM_SECTOR);
    bam = d64->data + offset;

    memcpy(bament,
           bam + ZCC_D64_BAM_TRACKS + (track - 1) * ZCC_D64_BAMENT_SIZE,
           ZCC_D64_BAMENT_SIZE);

    return true;
}


/** \brief  Determine number of blocks free on \a d64
 *
 * \param[in]   d64     D64 image
 *
 * \return  blocks free
 *
 * \todo    Mask out unused bits for invalid sectors
 */
int zcc_d64_blocks_free(zcc_d64_t *d64)
{
    int blocks = 0;

    for (int track = ZCC_D64_TRACK_MIN; track <= ZCC_D64_TRACK_MAX; track++) {
        /* exclude track 18 */
        if (track != ZCC_D64_DIR_TRACK) {
            uint8_t bament[ZCC_D64_BAMENT_SIZE];
            int popcount;

            zcc_d64_bament_read(d64, bament, track);

            popcount = zcc_popcount_byte(bament[ZCC_D64_BAMENT_BITMAP + 0])
                     + zcc_popcount_byte(bament[ZCC_D64_BAMENT_BITMAP + 1])
                     + zcc_popcount_byte(bament[ZCC_D64_BAMENT_BITMAP + 2]);
#if 0
            zcc_debug("popcount for track %d = %d\n", track, popcount);
#endif
            blocks += popcount;
        }
    }
    return blocks;
}


/** \brief  Initialize d64 dirent
 *
 * \param[out]  dirent  D64 director entry object
 */
void zcc_d64_dirent_init(zcc_d64_dirent_t *dirent)
{
    dirent->d64 = NULL;
    memset(dirent->name, 0, ZCC_D64_DISKNAME_MAXLEN);
    memset(dirent->geos, 0, ZCC_D64_DIRENT_GEOS_SIZE);
    dirent->filetype = 0;
    dirent->blocks = 0;
    dirent->track = 0;
    dirent->sector = 0;
    dirent->dir_track = 0;
    dirent->dir_sector = 0;
    dirent->rel_length = 0;
    dirent->ssb_track = 0;
    dirent->ssb_sector = 0;
}


/** \brief  Read data into \a dirent from \a data
 *
 * \param[out]  dirent  D64 directory entry
 * \param[in]   data    raw directory entry data
 */
void zcc_d64_dirent_read(zcc_d64_dirent_t *dirent, const uint8_t *data)
{
    /* $00 (useless) */
    dirent->dir_track = data[ZCC_D64_DIRENT_DIR_TRACK];
    /* $01 */
    dirent->dir_sector = data[ZCC_D64_DIRENT_DIR_SECTOR];
    /* $02 */
    dirent->filetype = data[ZCC_D64_DIRENT_FILETYPE];
    /* $03 */
    dirent->track = data[ZCC_D64_DIRENT_TRACK];
    /* $04 */
    dirent->sector = data[ZCC_D64_DIRENT_SECTOR];
    /* $05-$14 */
    memcpy(dirent->name, data + ZCC_D64_DIRENT_FILENAME, ZCC_CBMDOS_FILENAME_MAX);
    /* $15 */
    dirent->ssb_track = data[ZCC_D64_DIRENT_SSB_TRACK];
    /* $16 */
    dirent->ssb_sector = data[ZCC_D64_DIRENT_SSB_SECTOR];
    /* $17 */
    dirent->rel_length = data[ZCC_D64_DIRENT_REL_LENGTH];
    /* $18-$1d */
    memcpy(dirent->geos, data + ZCC_D64_DIRENT_GEOS, ZCC_D64_DIRENT_GEOS_SIZE);
    /* $1e-$1f */
    dirent->blocks = (uint8_t)(data[ZCC_D64_DIRENT_BLOCKS_LSB]
            + 256 * data[ZCC_D64_DIRENT_BLOCKS_MSB]);
}


/** \brief  Initialize d64 dirent iter
 *
 * \param[in,out]   iter    d64 dirent iter
 * \param[in]       d64     d64 image
 *
 * \return  true if at least on dirent was found, false otherwise
 */
bool zcc_d64_dirent_iter_init(zcc_d64_dirent_iter_t *iter, zcc_d64_t *d64)
{
    uint8_t buffer[ZCC_D64_BLOCK_SIZE_RAW];
    iter->d64 = d64;
    iter->sector = ZCC_D64_DIR_SECTOR;
    iter->offset = 0;
    iter->index = 0;

    /* read raw initial block at (18,1) */
    zcc_d64_block_read(d64, buffer, ZCC_D64_DIR_TRACK, ZCC_D64_DIR_SECTOR);
    /* convert to dirent */
    zcc_d64_dirent_read(&(iter->dirent), buffer);

    return (bool)(iter->dirent.name[0]);
}


bool zcc_d64_dirent_iter_next(zcc_d64_dirent_iter_t *iter)
{
    uint8_t buffer[ZCC_D64_BLOCK_SIZE_RAW];
    if (iter->index == ZCC_D64_DIRENT_MAX - 1
            || iter->dirent.name[0] == 0) {
        return false;
    }

    /* move to next entry */
    if (iter->offset < 0xe0) {
        /* move inside sector */
        iter->offset += ZCC_D64_DIRENT_SIZE;
    } else {
        /* check for next dir sector */
        long offset;
        uint8_t *data;
        int next_sector;


        zcc_debug("Checking for next dir sector\n");
        offset = zcc_d64_block_offset(ZCC_D64_DIR_TRACK, iter->sector);
        if (offset < 0) {
            return false;
        }
        data = iter->d64->data + offset;
        next_sector = data[1];
        zcc_debug("Next block = (18,%d)\n", next_sector);
        if (next_sector == 255) {
            return false;
        }
        iter->offset = 0;
        iter->sector = next_sector;
    }
    /* read raw initial block at (18,1) */
    zcc_debug("Reading dirent from (18,%d), offset %02x\n", iter->sector, iter->offset);
    zcc_d64_block_read(iter->d64, buffer, ZCC_D64_DIR_TRACK, iter->sector);
    /* convert to dirent */
    zcc_d64_dirent_read(&(iter->dirent), buffer + iter->offset);
    iter->index++;
    return (bool)(iter->dirent.name[0]);
}


/** \brief  Dump info on dirent \a iter on stdout
 *
 * \param[in]   iter    D64 dirent iter
 */
void zcc_d64_dirent_iter_dump(const zcc_d64_dirent_iter_t *iter)
{
    printf("dir sector = %d, in-sector offset: %02x\n",
            iter->sector, (unsigned int)(iter->offset));
}



/** \brief  Initialize D64 dir object
 */
void zcc_d64_dir_init(zcc_d64_dir_t *dir, zcc_d64_t *d64)
{
    dir->d64 = d64;
    memset(dir->diskname, 0, ZCC_D64_DISKNAME_MAXLEN);
    memset(dir->diskid, 0, ZCC_D64_DISKID_MAXLEN);
    for (int i = 0; i < ZCC_D64_DIRENT_MAX; i++) {
        zcc_d64_dirent_init(&(dir->entries[i]));
    }
    dir->entry_count = 0;
}


/** \brief  Read directory into \a dir
 *
 * The \a dir should have been initialized with zcc_d64_dir_init() beforehand.
 *
 * \param[in,out]   dir D64 directoru
 *
 * \return  bool
 */
bool zcc_d64_dir_read(zcc_d64_dir_t *dir)
{
    long offset;
    uint8_t *bam;
    zcc_d64_dirent_iter_t iter;

    /* get pointer to BAM */
    offset = zcc_d64_block_offset(18, 0);
    bam = dir->d64->data + offset;

    /* get disk name */
    memcpy(dir->diskname, bam + ZCC_D64_BAM_DISKNAME, ZCC_D64_DISKNAME_MAXLEN);
    /* get disk id */
    memcpy(dir->diskid, bam + ZCC_D64_BAM_DISKID, ZCC_D64_DISKID_MAXLEN);


    /* initialize dirent iter */
    if (!zcc_d64_dirent_iter_init(&iter, dir->d64)) {
        zcc_debug("NO dir entries.\n");
        return true;
    }

    /* iterate over entries */
    do {
        /* copy dirent */
        dir->entries[dir->entry_count++] = iter.dirent;
    } while (zcc_d64_dirent_iter_next(&iter));

    return true;
}


/** \brief  Dump directory \a dir on stdout
 *
 * \param[in]   dir D64 directory
 */
void zcc_d64_dir_dump(zcc_d64_dir_t *dir)
{
    char diskname_buf[ZCC_D64_DISKNAME_MAXLEN + 1];
    char diskid_buf[ZCC_D64_DISKID_MAXLEN + 1];

    zcc_pet_to_asc_str(diskname_buf, dir->diskname, ZCC_D64_DISKNAME_MAXLEN);
    zcc_pet_to_asc_str(diskid_buf, dir->diskname, ZCC_D64_DISKID_MAXLEN);

    printf("0 \"%16s\" %5s\n", diskname_buf, diskid_buf);

    for (int i = 0; i < dir->entry_count; i++) {
        zcc_d64_dirent_t *dirent = &(dir->entries[i]);
        char filename[ZCC_CBMDOS_FILENAME_MAX + 1];

        zcc_pet_to_asc_str(filename, dirent->name, ZCC_CBMDOS_FILENAME_MAX);

        printf("%-5d \"%s\" %c%s%c\n",
                (int)(dirent->blocks),
                filename,
                dirent->filetype & ZCC_CBMDOS_CLOSED_MASK ? ' ' : '*',
                zcc_cbmdos_filetype_str(dirent->filetype),
                dirent->filetype & ZCC_CBMDOS_LOCKED_MASK ? '<' : ' ');
    }
    printf("%d blocks free.\n", zcc_d64_blocks_free(dir->d64));
}
