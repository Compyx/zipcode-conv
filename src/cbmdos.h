/** \file   cbmdos.h
 * \brief   CBM-DOS generics - header
 *
 */

/*
 * This file is part of zipcode-conv
 *
 *  Copyright (C) 2020  Bas Wassink <b.wassink@ziggo.nl>
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
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.*
 */

#ifndef ZCC_CBMDOS_H
#define ZCC_CBMDOS_H

/** \brief  Maximum length of a CBM DOS filename in PETSCII
 */
#define ZCC_CBMDOS_FILENAME_MAX 16


/** \brief  File type enumerator
 */
typedef enum zcc_cbmdos_filetype_e {
    ZCC_CBMDOS_FILETYPE_DEL = 0,    /**< DELeted file */
    ZCC_CBMDOS_FILETYPE_SEQ,        /**< SEQuental file */
    ZCC_CBMDOS_FILETYPE_PRG,        /**< PRoGram file */
    ZCC_CBMDOS_FILETYPE_USR,        /**< USeR file */
    ZCC_CBMDOS_FILETYPE_REL         /**< RELative file */
} zcc_cbmdos_filetype_t;


/** \brief  Bitmask to filter out file type of a filetype byte
 */
#define ZCC_CBMDOS_FILETYPE_MASK    0x07

/** \brief  Bitmask to filter out the locked state of a filetype byte
 */
#define ZCC_CBMDOS_LOCKED_MASK      0x40

/** \brief  Bitmask to filter out the closed state of a filetype byte
 */
#define ZCC_CBMDOS_CLOSED_MASK      0x80


const char *zcc_cbmdos_filetype_str(zcc_cbmdos_filetype_t type);

#endif
