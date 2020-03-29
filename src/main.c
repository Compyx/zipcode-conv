/* vim: set et ts=4 sw=4 sts=4 fdm=marker syntax=c.doxygen: */

/** \file   main.c
 * \brief   Entry point
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


#include "d64.h"
#include "mem.h"


/** \brief  Test image for src/d64.c
 */
#define D64_TEST_IMAGE  "data/d64/gumbo_dec2019.d64"


/** \brief  Command line handler of tool
 *
 * \return  EXIT_SUCCESS or EXIT_FAILURE
 */
int main(void)
{

    zcc_d64_t d64;

    printf("Hello World!\n");

    printf("Reading D64 '%s' ... ", D64_TEST_IMAGE);
    zcc_d64_init(&d64);
    if (zcc_d64_read(&d64, D64_TEST_IMAGE, 0)) {
        printf("OK, %ld bytes\n", (unsigned long)d64.size);
        zcc_d64_dump_info(&d64);
        zcc_d64_dump_bam(&d64);

        zcc_d64_free(&d64);
    } else {
        printf("Failed.\n");
    }

    return EXIT_SUCCESS;
}
