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


/** \brief  D64 handle
 *
 */
typedef struct zcc_d64_s {
    char *          path;   /**< path to image file */
    uint8_t *       data;   /**< binary data */
    size_t          size;   /**< size of data */
    zcc_d64_type_t  type;   /**< DOS type */
} zcc_d64_t;


void zcc_d64_init(zcc_d64_t *d64);
void zcc_d64_free(zcc_d64_t *d64);
bool zcc_d64_read(zcc_d64_t *d64, const char *path, zcc_d64_type_t type);
void zcc_d64_dump_info(const zcc_d64_t *d64);

#endif

