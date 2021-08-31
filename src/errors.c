/** \file   errors.c
 * \brief   Error reporting
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
#include <errno.h>

#include "errors.h"


/** \brief  Library/tool-wide error code
 */
int zcc_errno;


/** \brief  Error messages
 */
static const char *err_msgs[] = {
    "OK",
    "invalid use of NULL pointer",
    "I/O error",
    "track number out of range",
    "sector number out of range",
    "invalid filename",
    "RLE error",
    "invalid zipcode data",
    "invalid zipcode pack method"
};


/** \brief  Get error message for error number \a code
 *
 * \param[in]   code    error code
 *
 * \return  error message
 */
const char *zcc_strerror(int code)
{
    if (code >= 0 || code < (int)(sizeof err_msgs / sizeof err_msgs[0])) {
        return err_msgs[code];
    } else {
        return "unknown error";
    }
}


/** \brief  Print error code and message on stderr
 *
 * \param[in]   prefix  optional prefix
 */
void zcc_perror(const char *prefix)
{
    if (prefix != NULL && *prefix != '\0') {
        fprintf(stderr, "%s: ", prefix);
    }
    fprintf(stderr, "(%d) %s", zcc_errno, zcc_strerror(zcc_errno));
    if (errno != 0) {
        fprintf(stderr, ": (%d) %s", errno, strerror(errno));
    }
    fputc('\n', stderr);
}

