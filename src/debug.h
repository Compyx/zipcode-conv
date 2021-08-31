/** \file   debug.h
 * \brief   Debug messages
 * \ingroup base
 */

#ifndef ZCC_DEBUG_H
#define ZCC_DEBUG_H


#ifdef DEBUG_ZCC

# include <stdio.h>
# include <stdlib.h>
# include <stdarg.h>
# include <inttypes.h>
# include <stdbool.h>

/** \brief  Print debug message stdout when --enable-debug is given
 *
 * Prints '[debug] file:lineno:func(): message' with a newline after message.
 * Works like printf()
 */
# define zcc_debug(...) \
    fprintf(stderr, \
            "[debug] %s:%d:%s(): ", \
            __FILE__, __LINE__, __func__); \
    fprintf(stderr, __VA_ARGS__); \
    fprintf(stderr, "\n");

#else
  /** \brief    Debug message stub (NOP)
   */
# define zcc_debug(...)
#endif


/* Enable commandline parsing debugging when requested
 *
 * This requires --enable-debug for now.
 */
#ifdef DEBUG_CMDLINE
  /** \brief    Cmdline: print debugging info
   */
# define zcc_debug_cmdline zcc_debug
#else
  /** \brief    Cmdline: print debugging info (NOP)
   */
# define zcc_debug_cmdline(...)
#endif


/* Enable unittest debugging when requested
 *
 * This requires --enable-debug for now.
 */
#ifdef DEBUG_UNITTEST
  /** \brief    Unittest: print debugging info
   */
# define zcc_debug_unittest zcc_debug
#else
  /** \brief    Unittest: print debugging info (NOP)
   */
# define zcc_debug_unittest(...)
#endif

#endif
