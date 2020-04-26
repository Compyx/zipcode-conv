/* vim: set et ts=4 sw=4 sts=4 fdm=marker syntax=c.doxygen: */

/** \file   test_d64.c
 * \brief   Test D64 handling
 */


#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "unit.h"

#include "../src/d64.h"
#include "../src/errors.h"

#define ARMALYTE    "data/d64/armalyte-rem.d64"


/*
 * Forward declarations
 */

static bool setup(void);
static bool teardown(void);

static bool test_d64_read(int *, int *);


/** \brief  Test cases
 */
static unit_test_t tests[] = {
    { "read", "Test reading a D64 image",
        test_d64_read, false },
    { NULL, NULL, NULL, NULL }
};


/** \brief  Module containing tests
 */
unit_module_t d64_module = {
    "d64",
    "Tests for the D64 code",
    setup, teardown,
    0, 0,
    tests
};


/** \brief  Setup function
 *
 * This is called when initializing the module.
 *
 * Currently doesn't do anything, except report it got called (with --debug)
 * and return `true`.
 *
 * \return  true
 */
static bool setup(void)
{
    printf("%s:%d:%s(): called.\n",
            __FILE__, __LINE__, __func__);
    return true;
}


/** \brief  Teardown function
 *
 * This is called when cleaning up any resources set by #setup.
 *
 * Doesn't do anything except reporting it get called (with --debug) and
 * return `true`
 *
 * \return  true
 */
static bool teardown(void)
{
    printf("%s:%d:%s(): called.\n",
            __FILE__, __LINE__, __func__);
    return true;
}



static bool test_d64_read(int *total, int *passed)
{
    zcc_d64_t d64;
    bool result;

    (*total)++;
    printf(".. Loading '%s' ... ", ARMALYTE);
    zcc_d64_init(&d64);

    result = zcc_d64_read(&d64, ARMALYTE, 0);
    if (result) {
        printf("OK.\n");
        (*passed)++;
    } else {
        printf("failed:\n");
        zcc_perror(__func__);
    }
    zcc_d64_free(&d64);
    return result;
}
