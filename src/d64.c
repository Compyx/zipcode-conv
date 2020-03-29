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


