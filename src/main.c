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
#include "io.h"
#include "mem.h"
#include "zipdisk.h"


/** \brief  Test image for src/d64.c
 */
#define D64_TEST_IMAGE  "data/d64/gumbo_dec2019.d64"

#define ZIPDISK_TEST_IMAGE  "data/zipdisk/1!SPHERE.Z64"


/** \brief  Test the zcc_basename() function
 */
static void test_zcc_basename(void)
{
    static char *tests[] = {
        "foo", "", "foo/bar", "foo/bar/huppel"
    };

    size_t i;

    printf("Testing zcc_basename():\n");
    for (i = 0; i < sizeof tests / sizeof tests[0]; i++) {
        printf(".. basename('%s') = '%s'\n",
                tests[i], zcc_basename(tests[i]));
    }
}



static void test_zipdisk(void)
{
    zcc_zipdisk_t zip;



    printf("Testing zipdisk code:\n");

    zcc_zipdisk_test_iter(ZIPDISK_TEST_IMAGE);

    zcc_zipdisk_init(&zip);
    if (!zcc_zipdisk_read(&zip, ZIPDISK_TEST_IMAGE)) {
        fprintf(stderr, "OOPS\n");
        return;
    }

    zcc_zipdisk_unzip(&zip, "sphere-output.d64");
    zcc_zipdisk_free(&zip);

#if 0
    zcc_zipdisk_init(&zip);
    if (zcc_zipdisk_read(&zip, ZIPDISK_TEST_IMAGE)) {
        for (int i = 0; i < zip.slice_count; i++) {
            zcc_zipdisk_dump_slice(&zip, i);
        }
    }

    zcc_zipdisk_free(&zip);
#endif
}


/** \brief  Command line handler of tool
 *
 * \return  EXIT_SUCCESS or EXIT_FAILURE
 */
int main(void)
{

    zcc_d64_t d64;


    printf("Hello World!\n");

    test_zcc_basename();

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


    test_zipdisk();

    return EXIT_SUCCESS;
}
