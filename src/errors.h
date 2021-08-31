/** \file   errors.h
 * \brief   Error reporting - header
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

#ifndef ZCC_ERRORS_H
#define ZCC_ERRORS_H

/** \brief  Error codes
 */
enum zcc_errno_e {
    ZCC_ERR_OK = 0,             /**< OK */
    ZCC_ERR_NULL,               /**< invalid use of null */
    ZCC_ERR_IO,                 /**< libc I/O error */
    ZCC_ERR_TRACK_RANGE,        /**< track number out of range */
    ZCC_ERR_SECTOR_RANGE,       /**< sector number out of range */
    ZCC_ERR_INVALID_FILENAME,   /**< invalid filename */
    ZCC_ERR_RLE,                /**< RLE error (probably need to split this
                                     into multiple errors) */
    ZCC_ERR_ZC_INVALID_DATA,        /**< invalid zipcode data */
    ZCC_ERR_ZC_INVALID_PACK_METHOD  /**< invalid zipcode pack method (%11) */
};

extern int zcc_errno;

const char *zcc_strerror(int code);
void zcc_perror(const char *prefix);

#endif
