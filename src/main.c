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
#include <string.h>

#include "cmdline.h"
#include "d64.h"
#include "io.h"
#include "mem.h"
#include "zipdisk.h"



/** \brief  Test image for src/d64.c
 */
#define D64_TEST_IMAGE  "data/d64/gumbo_dec2019.d64"

/** \brief  Test image for src/zipdisk.c
 *
 * This is a correct archive, so proper code should not fail on this archive
 */
#define ZIPDISK_TEST_IMAGE  "data/zipdisk/1!SPHERE.Z64"


#if 0
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
#endif

#if 0
/** \brief  Run some tests on src/zipdisk.c
 *
 * Should be removed, and proper tests added to ./tests/
 */
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
#endif

static int opt_zipdisk_info = 0;
static int opt_zipdisk_unzip = 0;
static int opt_verbose = 0;



/*
 * Commands
 *
 *
 */

/** \brief  Show brief information on a zipdisk archive
 *
 * \return bool
 */
static bool cmd_zipdisk_info(strlist_t *args)
{
    char *zip = strlist_get(args, 0);

    if (zip != NULL) {
        return zcc_zipdisk_show_info(zip, opt_verbose);
    } else {
        return false;
    }
}


static bool cmd_zipdisk_unzip(strlist_t *args)
{
    char *infile = strlist_get(args, 0);
    char *outfile = strlist_get(args, 1);
    zcc_zipdisk_t zip;
    char *bname;
    bool outfile_alloced = false;

    if (infile == NULL) {
        fprintf(stderr, "missing argument\n");
        return false;
    }

    /* either use arg[1] or use arg[0] without the '1!' */
    if (outfile == NULL) {
        size_t blen;

        fprintf(stderr, "TODO: generate D64 filename from zipdisk filename\n");

        bname = zcc_basename(infile);
        blen = strlen(bname);
        printf("basename = '%s'\n", bname);
        /* -2 for '[1-5]!', + 4 for .d64, +1 for '\0' */
        outfile = zcc_malloc(blen - 2 + 4 + 1);
        memcpy(outfile, bname + 2, blen - 2);
        memcpy(outfile + blen - 2, ".d64", 4);

        outfile_alloced = true;


    }

    printf("infile  = '%s'\n", infile);
    printf("outfile = '%s'\n", outfile);

    zcc_zipdisk_init(&zip);
    if (!zcc_zipdisk_read(&zip, infile)) {
        fprintf(stderr, "fuck\n");
        if (outfile_alloced) {
            zcc_free(outfile);
        }
        return false;
    }
    if (zcc_zipdisk_unzip(&zip, outfile)) {
        fprintf(stderr, "OK.\n");
    } else {
        fprintf(stderr, "fick");
        if (outfile_alloced) {
            zcc_free(outfile);
        }
        return false;
    }

    if (outfile_alloced) {
         zcc_free(outfile);
    }
    return true;
}


static const cmdline_option_t main_cmdline_options[] = {
    { 0, "zipdisk-info", NULL, CMDLINE_TYPE_BOOL,
        &opt_zipdisk_info, NULL, "show info on zipdisk archive" },
    { 0, "verbose", NULL, CMDLINE_TYPE_BOOL,
        &opt_verbose, 0, "enable verbose output" },
    { 0, "zipdisk-unzip", NULL, CMDLINE_TYPE_BOOL,
        &opt_zipdisk_unzip, NULL, "unpack" },

    CMDLINE_OPTION_TERMINATOR
};

static bool handle_commands(strlist_t *args)
{
    if (opt_zipdisk_info) {
        return cmd_zipdisk_info(args);
    } else if (opt_zipdisk_unzip) {
        return cmd_zipdisk_unzip(args);
    }

    return true;
}



/** \brief  Command line handler of tool
 *
 * \return  EXIT_SUCCESS or EXIT_FAILURE
 */
int main(int argc, char *argv[])
{
    int result;
    strlist_t *args = NULL;
    int retval = EXIT_SUCCESS;

    /* initialize command line parser */
    cmdline_init("zipcode-conv", "0.1.0");
    cmdline_add_options(main_cmdline_options);

#if DEBUG_ZCC
    int i;
    printf("argc = %d, argv= [\n", argc);
    for (i = 0; i < argc; i++) {
        printf("    '%s'\n", argv[i]);
    }
    printf("]\n");
#endif

    result = cmdline_parse(argc, argv, &args);
    switch (result) {
        case CMDLINE_EXIT_HELP: /* fall through */
        case CMDLINE_EXIT_VERSION:
            break;
        case CMDLINE_EXIT_ERROR:
            retval = EXIT_FAILURE;
            break;
        case CMDLINE_EXIT_OK:

            if (handle_commands(args)) {
                printf("OK\n");
            } else {
                printf("failed\n");
                retval = EXIT_FAILURE;
            }
            break;
        default:
            fprintf(stderr, "%s: unknown cmdline parser exit code %d.\n",
                    argv[0], result);
            break;
    }

    cmdline_exit();

    return retval;
}
