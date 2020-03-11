/* vim: set et ts=4 sw=4 sts=4 fdm=marker syntax=c.doxygen: */

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


enum {
    ZCC_ERR_OK = 0,
    ZCC_ERR_IO
};

extern int zcc_errno;

const char *zcc_strerror(int code);
void zcc_perror(const char *prefix);

#endif
