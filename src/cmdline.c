/* vim: set et ts=4 sw=4 sts=4 fdm=marker syntax=c.doxygen: */

/** \file   cmdline.c
 * \brief   Commandline handling
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#if 0
#include "convert.h"
#endif
#include "debug.h"
#include "mem.h"
#include "strlist.h"

#include "cmdline.h"


/** \brief  Convert pointer \a P to int
 *
 * \param[in]   P   pointer value
 */
#define base_ptr_to_int(P) ((int)(intptr_t)(P))





/** \brief  Initial size of the options list array when allocating
 *
 * \note    Currently set to a rediculously small value to test the code
 *          handling the resizing of the list.
 */
#define OPT_LIST_INITIAL_SIZE   4


/*
 * Forward declarations of static functions
 */

static void cmdline_option_list_init(void);
static void cmdline_option_list_free(void);
static void cmdline_option_free(cmdline_option_t *option);
static cmdline_option_t *cmdline_option_init(const cmdline_option_t *option);
static bool cmdline_add_option(const cmdline_option_t *option);


/** \brief  Program name
 *
 * Used for display during --help, --version and error message
 */
static char *prg_name = NULL;


/** \brief  Program version
 *
 * Used for displaying --version
 */
static char *prg_version = NULL;


/** \brief  Prologue function to call for --help (optional)
 */
static void (*prologue)(void) = NULL;


/** \brief  Epilogue function to call for --help (optional)
 *
 * Called after listing the register command line options
 */
static void (*epilogue)(void) = NULL;


/** \brief  List of registered options
 */
static cmdline_option_t **option_list = NULL;


/** \brief  Size of the \c options_list
 */
static size_t option_list_size = 0;


/** \brief  Number of used entries in the \c options_list
 */
static size_t option_list_used = 0;

/** \brief  Pointer to the strlist_t passed to cmdline_parse()
 *
 * Used to free the arguments collected by the parser when calling
 * cmdline_exit(). To keep the argument list, use strlist_dup() (TODO).
 */
static strlist_t **args_list;


/** \brief  Initialize the command line options list
 *
 * Allocates a list of cmdline_option_t pointers.
 *
 * The list is filled with `NULL` pointers initialy.
 *
 * \see #OPT_LIST_INITIAL_SIZE
 */
static void cmdline_option_list_init(void)
{
    size_t i;

    option_list = zcc_malloc(OPT_LIST_INITIAL_SIZE * sizeof *option_list);
    option_list_size = OPT_LIST_INITIAL_SIZE;
    option_list_used = 0;
    for (i = 0; i < OPT_LIST_INITIAL_SIZE; i++) {
        option_list[i] = NULL;
    }
}


/** \brief  Free memory used by the command line list and its elements
 *
 * Frees all command line option objects registered and their members and the
 * list itself.
 */
static void cmdline_option_list_free(void)
{
    size_t i;

    for (i = 0; i < option_list_used; i++) {
        cmdline_option_free(option_list[i]);
    }
    zcc_free(option_list);
}


/** \brief  Add \a option to the command line option list
 *
 * \param[in]   option  command line option
 */
static void cmdline_option_list_add(cmdline_option_t *option)
{
    /* do we need to resize the option list? */
    if (option_list_size == option_list_used) {
        /* yup */
        option_list_size *= 2;
        option_list = zcc_realloc(option_list,
                option_list_size * sizeof *option_list);
    }
    option_list[option_list_used++] = option;
}


/** \brief  Deallocate \a option and its members
 *
 * \param[in,out]   option  command line option
 */
static void cmdline_option_free(cmdline_option_t *option)
{
    char **tmp;
    strlist_t **lptr;

    switch (option->type) {
        case CMDLINE_TYPE_BOOL: /* fall through */
        case CMDLINE_TYPE_INT:
            /* NOP */
            break;
        case CMDLINE_TYPE_STR:
            tmp = option->target;
            if (*tmp != NULL) {
                zcc_free(*tmp);
            }
            break;
        case CMDLINE_TYPE_ARR:
            /* free list of strings, if any */
            lptr = option->target;
            if (*lptr != NULL) {
                strlist_free(*lptr);
            }
            break;
        default:
            break;
    }
    zcc_free(option);
}


/** \brief  Copy \a option and initialize the copy
 *
 * \param[in]   option  command line option
 *
 * \return  initialized copy of \a option, or `NULL` on error
 */
static cmdline_option_t *cmdline_option_init(const cmdline_option_t *option)
{
    cmdline_option_t *newopt = zcc_malloc(sizeof *newopt);

    int *iptr;
    char **sptr;
    strlist_t **lptr;

    /* copy members */
    newopt->short_opt = option->short_opt;
    newopt->long_opt = option->long_opt;
    newopt->arg_desc = option->arg_desc;
    newopt->type = option->type;
    newopt->target = option->target;
    newopt->factory = option->factory;
    newopt->description = option->description;

    /* initialize target with factory value */
    switch (option->type) {
        case CMDLINE_TYPE_BOOL:
            iptr =newopt->target;
            *iptr = base_ptr_to_int(newopt->factory);
            break;
        case CMDLINE_TYPE_INT:
            iptr = newopt->target;
            *iptr = base_ptr_to_int(newopt->factory);
            break;
        case CMDLINE_TYPE_STR:
            sptr = newopt->target;
            if (newopt->factory != NULL) {
                *sptr = zcc_strdup((const char *)(newopt->factory));
            } else {
                *sptr = NULL;
            }
            break;
        case CMDLINE_TYPE_ARR:
            /* TODO: initialize string list and store it */
            zcc_debug_cmdline("TODO: allocate string list");
            lptr = newopt->target;
            *lptr = strlist_init();
            break;
        default:
            zcc_debug_cmdline("illegal option type %d'\n", newopt->type);
            cmdline_option_free(newopt);
            newopt = NULL;
    }

    return newopt;
}



/** \brief  Initialize command line parser
 *
 * \param[in]   name    program name
 * \param[in]   version program version
 */
void cmdline_init(const char *name, const char *version)
{
    prg_name = zcc_strdup(name);
    prg_version = zcc_strdup(version);

    cmdline_option_list_init();
}


/** \brief  Clean up command line parser
 */
void cmdline_exit(void)
{
    zcc_free(prg_name);
    zcc_free(prg_version);

    cmdline_option_list_free();
    strlist_free(*args_list);
    *args_list = NULL;
}


/** \brief  Set prologue function to call for --help
 *
 * Called in handling of --help after outputting "Usage: <prg_name> ..." but
 * before listing the registers options.
 *
 * \param[in]   func    pointer to prologue function
 */
void cmdline_set_prologue(void (*func)(void))
{
    prologue = func;
}


/** \brief  Set epilogue function to call for --help
 *
 * Called after listing the register command line options.
 *
 * \param[in]   func    pointer to epilogue function
 */
void cmdline_set_epilogue(void (*func)(void))
{
    epilogue = func;
}


/** \brief  Show program name and version on stdout
 */
void cmdline_show_version(void)
{
    printf("%s %s\n", prg_name, prg_version);
}


/** \brief  Add \a option to the command line option list
 *
 * \param[in]   option  command line option
 *
 * \return  `true` on success, `false` when \a option wasn't correct
 */
static bool cmdline_add_option(const cmdline_option_t *option)
{
    cmdline_option_t *newopt = cmdline_option_init(option);

    if (newopt == NULL) {
        return false;
    }
    cmdline_option_list_add(newopt);
    return true;
}


/** \brief  Add a list of command line options to the command line parser
 *
 * \param[in]   options list of command line options
 *
 * \note    The list needs be terminated by #CMDLINE_OPTION_TERMINATOR
 *
 * \return  bool
 */
bool cmdline_add_options(const cmdline_option_t *options)
{
    const cmdline_option_t *opt;

    for (opt = options;
            opt->short_opt > 0
            || (opt->long_opt != NULL && *(opt->long_opt) != '\0');
            opt++)
    {
        zcc_debug_cmdline("Adding option -%c/--%s (%s)\n",
                opt->short_opt, opt->long_opt, opt->description);
        if (!cmdline_add_option(opt)) {
            return false;
        }
    }
    return true;
}


/** \brief  List registered command line options
 */
void cmdline_list_options(void)
{
    size_t i;

    for (i = 0; i < option_list_used; i++) {
        cmdline_option_t *option = option_list[i];
        int sopt = option->short_opt;
        const char *lopt = option->long_opt;
        cmdline_option_type_t type = option->type;
        const char *arg_desc = option->arg_desc;
        int c;


        if (sopt > 0 && lopt != NULL) {
            c = printf("   -%c, --%s", sopt, lopt);
        } else if (sopt > 0 && lopt == NULL) {
            c = printf("   -%c", sopt);
        } else {
            c = printf("       --%s", lopt);
        }
        /* add argument description */
        if (type != CMDLINE_TYPE_BOOL) {
            c += printf(" %s", arg_desc);
        }

        while (c < 40) {
            putchar(' ');
            c++;
        }
        printf("%s\n", option->description);
    }
}


/** \brief  Show help on stdout
 *
 * \todo    Add support for prologue and epilogue
 */
void cmdline_show_help(void)
{
    printf("Usage: %s [<options>] [<args>]\n\n", prg_name);
    if (prologue != NULL) {
        prologue();
    }
    cmdline_list_options();
    if (epilogue != NULL) {
        epilogue();
    }
}


/** \brief  Look up option in \a arg
 *
 * \param[in]   arg command line argument starting with a dash
 *
 * \return  option object when found, `NULL` on failure
 */
static cmdline_option_t *option_find(const char *arg)
{
    size_t i;

    for (i = 0; i < option_list_used; i++) {
        cmdline_option_t *option = option_list[i];

        if (arg[1] == '-') {
            /* long opt */
            if (strcmp(arg + 2, option->long_opt) == 0) {
                return option;
            }
        } else {
            zcc_debug_cmdline("arg[1]= '%c', option->short_opt = '%c'\n",
                    arg[1], option->short_opt);
            if (arg[1] == option->short_opt) {
                return option;
            }
        }
    }
    return NULL;
}


/** \brief  Handle \a option
 *
 * \param[in,out]   option  option pointer
 * \param[in]       arg     optional option argument
 *
 * \return  delta in argv, or -1 on error
 */
static int option_handle(cmdline_option_t * option, const char *arg)
{
    long num;
    char *endptr;
    int delta = 0;  /* arg consumed (ie non-bool) or -1 on error */
    char **sptr;

#ifdef HAVE_DEBUG
    if (option->short_opt > 0 && option->long_opt != NULL) {
        zcc_debug_cmdline("handling option -%c/--%s",
                option->short_opt, option->long_opt)
    } else if (option->short_opt > 0) {
        zcc_debug_cmdline("handling option -%c", option->short_opt)
    } else {
        zcc_debug_cmdline("handling option --%s", option->long_opt);
    }
#endif

    zcc_debug_cmdline("arg to option: '%s'", arg);

    /* check for option argument */
    if (option->type != CMDLINE_TYPE_BOOL &&
            (arg == NULL || *arg == '\0')) {
        fprintf(stderr, "%s: error: missing argument.\n", prg_name);
        return -1;
    }

    switch (option->type) {

        /* boolean */
        case CMDLINE_TYPE_BOOL:
            *((int *)(option->target)) = 1;
            break;

        /* integer */
        case CMDLINE_TYPE_INT:
           num = strtol(arg, &endptr, 0);
            if (*endptr != '\0') {
                /* failed, todo: report error */
                delta = -1;
            } else {
                *((int *)(option->target)) = (int)num;
                delta = 1;
            }
            break;

        /* string */
        case CMDLINE_TYPE_STR:
            /* free factory value, if available */
            sptr = (char **)(option->target);
            if (*sptr != NULL) {
                zcc_free(*sptr);
            }
            *sptr = zcc_strdup(arg);
            delta = 1;
            break;

        /* array */
        case CMDLINE_TYPE_ARR:
            strlist_add(*(strlist_t **)(option->target), arg);
            delta = 1;
            break;

        /* default: error out */
        default:
            zcc_debug_cmdline("Unsupported CMDLINE_TYPE %d", option->type);
            delta = -1;
            break;


    }

    return delta;
}


/** \brief  Parse command line arguments for options and arguments
 *
 * \param[in]   argc    argument count
 * \param[in]   argv    argument count
 * \param[out]  list    object to store reference to list of non-option args
 *
 * \note    The \a list is freed when calling cmdline_exit(), so if the list
 *          of args needs to be kept after calling cmdline_exit(), use the
 *          strlist_t function strlist_dup().
 *
 * \return  >= 0 on success, -1 on failure
 */
int cmdline_parse(int argc, char *argv[], strlist_t **list)
{
    int i;

#ifdef HAVE_DEBUG
    /* show input */
    zcc_debug_cmdline("argc = %d\n", argc);
    for (i = 0; i < argc; i++) {
        zcc_debug_cmdline("argv[%d] = '%s'\n", i, argv[i]);
    }
#endif

    /* store pointer to string list so we can clean it up on calling
     * cmdline_exit()
     */
    args_list = list;
    /* initialize list of non-option args */
    *list = strlist_init();


    for (i = 1; i < argc; i++) {
        int delta = 0;
        zcc_debug_cmdline("parsing argv[%d]: '%s'", i, argv[i]);
        if (argv[i][0] == '-') {
            /* possible option */
            cmdline_option_t *option;

            zcc_debug_cmdline(".. found possible option '%s'", argv[i]);

            /*
             * check --help and --version first
             */
            if (argv[i][1] == '-') {
                if (strcmp(argv[i], "--help") == 0) {
                    zcc_debug_cmdline(".... --help requested");
                    cmdline_show_help();
                    return CMDLINE_EXIT_HELP;
                } else if (strcmp(argv[i], "--version") == 0) {
                    zcc_debug_cmdline(".... --version requested");
                    cmdline_show_version();
                    return CMDLINE_EXIT_VERSION;
                }
            }

            option = option_find(argv[i]);
            if (option == NULL) {
                fprintf(stderr, "%s: unknown option '%s'\n", prg_name, argv[i]);
                return CMDLINE_EXIT_ERROR;
            }
            zcc_debug_cmdline(".. recognized option '%s'", argv[i]);
            delta = option_handle(option, argv[i + 1]);
            if (delta < 0) {
                return CMDLINE_EXIT_ERROR;
            }
        } else {
            /* non-option argument */
            zcc_debug_cmdline(".. adding non-option argument '%s'", argv[i]);
            strlist_add(*list, argv[i]);
        }
        i += delta;
    }

    return CMDLINE_EXIT_OK;
}
