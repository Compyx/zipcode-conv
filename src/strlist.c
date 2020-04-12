/* vim: set et ts=4 sw=4 sts=4 fdm=marker syntax=c.doxygen: */

/** \file   strlist.c
 * \brief   Heap-allocated list of strings
 * \ingroup base
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "debug.h"
#include "mem.h"

#include "strlist.h"


/** \brief  Initial size of a string list
 */
#define STRLIST_INITIAL_SIZE    4


/** \brief  Create a string list
 *
 * \return  new string list
 */
strlist_t *strlist_init(void)
{
    strlist_t *slist = zcc_malloc(sizeof *slist);
    slist->items = zcc_malloc(STRLIST_INITIAL_SIZE * sizeof *(slist->items));
    slist->items_max = STRLIST_INITIAL_SIZE;
    slist->items_used = 0;

    for (size_t i = 0; i < STRLIST_INITIAL_SIZE; i++) {
        slist->items[i] = NULL;
    }
    return slist;
}


/** \brief  Clean up memory used by \a slist
 *
 * \param[in,out]   slist   string list
 */
void strlist_free(strlist_t *slist)
{
    for (size_t i = 0; i < slist->items_used; i++) {
        zcc_free(slist->items[i]);
    }
    zcc_free(slist->items);
    zcc_free(slist);
}


/** \brief  Get number of items in \a slist
 *
 * \param[in]   slist   string list
 *
 * \return  number of items in the list
 */
size_t strlist_num_items(const strlist_t *slist)
{
    return slist->items_used;
}


/** \brief  Add string \a s to string list \a slist
 *
 * \param[in,out]   slist   string list
 * \param[in]       s       string to add
 */
void strlist_add(strlist_t *slist, const char *s)
{
    if (s == NULL) {
        zcc_debug("got NULL, ignoring.");
        return;
    }
    if (*s == '\0') {
        zcc_debug("got empty string, ignoring.");
        return;
    }

    if (slist->items_max == slist->items_used) {
        slist->items_max *= 2;
        zcc_debug("Resizing string list to %zu items",
                slist->items_max);
        slist->items = zcc_realloc(slist->items,
                                    slist->items_max * sizeof *(slist->items));
    }
    zcc_debug("adding item '%s' at index %zu.\n",
            s, slist->items_used);
    slist->items[slist->items_used++] = zcc_strdup(s);
}


/** \brief  Debug hook: dump contents of \a slist on stdout
 *
 * \param[in]   slist   string list
 */
void strlist_dump(const strlist_t *slist)
{
    for (size_t i = 0; i < slist->items_used; i++) {
        printf("[%zu] = '%s'\n", i, slist->items[i]);
    }
}


/** \brief  Create a deep copy of \a slist
 *
 * \param[in]   slist   string list
 *
 * \return  heap-allocated deep copy of \a slist
 */
strlist_t *strlist_dup(const strlist_t *slist)
{
    strlist_t *newlist = zcc_malloc(sizeof *newlist);
    newlist->items = zcc_malloc(slist->items_max * sizeof *(newlist->items));
    newlist->items_max = slist->items_max;
    newlist->items_used = slist->items_used;

    for (size_t i = 0; i < newlist->items_used; i++) {
        newlist->items[i] = zcc_strdup(slist->items[i]);
    }
    return newlist;
}
