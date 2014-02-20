 /******************************************************************************/
/*                                                                            */
/* FILE        : util.h                                                       */
/* AUTHOR      : Joshua Miller                                                */
/* DESCRIPTION : General utility functions and macros                         */
/*                                                                            */
/******************************************************************************/

#ifndef _UTIL_H
#define _UTIL_H

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include <pthread.h>
#include <setjmp.h>
#include <execinfo.h>

#define MAX_DESCRIPTOR 1028

#define __red__ "\033[1;31m"
#define __lgr__ "\033[1;32m"
#define __nrm__ "\033[0m"

#define STR(_x)   #_x
#define XSTR(_x)  STR(_x)

#ifdef DEBUG
#undef DEBUG
#endif

jmp_buf __ex_loc__;
jmp_buf __seg_loc__;
jmp_buf __to_loc__;

int __timeout__;
int __catching_exit__;
int __catching_segfault__;

char __fname__[MAX_DESCRIPTOR];
char __tname__[MAX_DESCRIPTOR];

int uerr;

#define MAX(a,b) \
    ({ __typeof__ (a) _a = (a); \
    __typeof__ (b) _b = (b); \
    _a > _b ? _a : _b; })

#define MIN(a,b) \
    ({ __typeof__ (a) _a = (a); \
    __typeof__ (b) _b = (b); \
    _a > _b ? _b : _a; })

#define INFO_DUMP(msg)                                  \
    do {                                                \
        fprintf(stderr, "%s:%d: %s: %s: ",              \
                __FILE__,                               \
                __LINE__,                               \
                __func__,                               \
                msg);                                   \
    } while(0)                                          

#ifndef _DEBUG
#define DEBUG(fmt, ...)
#else
#define DEBUG(fmt, ...)                         \
    do {                                        \
        INFO_DUMP("debug");                     \
        fprintf(stderr, fmt, ##__VA_ARGS__);    \
        fprintf(stderr, "\n");                  \
    } while (0)
#endif

#define ERROR(fmt, ...)                         \
    do {                                        \
        INFO_DUMP(__red__ "error" __nrm__);     \
        fprintf(stderr, fmt, ##__VA_ARGS__);    \
        fprintf(stderr, "\n");                  \
        exit(EXIT_FAILURE);                     \
    } while(0)

#define WARN(fmt, ...)                          \
    do {                                        \
        INFO_DUMP("warning");                   \
        fprintf(stderr, fmt, ##__VA_ARGS__);    \
        fprintf(stderr, "\n");                  \
    } while(0)

#define WARN_IF(f, fmt, ...){                   \
    do {                                        \
        if ((f) != 0){                          \
            WARN(fmt, ##__VA_ARGS__);           \
        }                                       \
    } while(0)

#define ERROR_IF(f, fmt, ...)                   \
    do {                                        \
        if ((f) != 0) {                         \
            ERROR(fmt, ##__VA_ARGS__);          \
        }                                       \
    } while(0)

#define SUCCESS(fmt, ...)                       \
    do {                                        \
        INFO_DUMP(__lgr__ "success" __nrm__);   \
        fprintf(stderr, fmt, ##__VA_ARGS__);    \
        fprintf(stderr, "\n");                  \
    } while(0)

#define SUCCESS_IF(condition, fmt, ...)         \
    do {                                        \
        if ((condition) != 0){                  \
            SUCCESS(fmt, ##__VA_ARGS__);        \
        }                                       \
    } while(0)


#define TRY do{ jmp_buf ex_buf__; if( !setjmp(ex_buf__) ){
#define CATCH } else {
#define ETRY } }while(0)
#define THROW longjmp(ex_buf__, 1)

#undef exit

#define exit(i)                                                 \
    do                                                          \
        {                                                       \
            if (__catching_exit__){                             \
                INFO_DUMP(__red__ "caught exit" __nrm__);       \
                fprintf(stderr, "with status %d\n", i);         \
                uerr = i;                                       \
                longjmp(__ex_loc__, 1);                         \
            } else {                                            \
                (exit)(i);                                      \
            }                                                   \
        }                                                       \
    while (0)                                                   \


#define CATCH_EXIT(f)                           \
    do {                                        \
        __catching_exit__ = 1;                  \
        if (!setjmp(__ex_loc__))                \
            (f);                                \
        __catching_exit__ = 0;                  \
    } while (0)                                 \

#define CATCH_ALL(f)                                    \
    do {                                                \
        signal(SIGSEGV, sig_handler);                   \
        snprintf(__fname__, MAX_DESCRIPTOR, STR(f));    \
        __catching_exit__ = 1;                          \
        __catching_segfault__ = 1;                      \
        uerr = 0;                                       \
        if (!setjmp(__seg_loc__) &&                     \
            !setjmp(__ex_loc__))                        \
            (f);                                        \
        if (uerr){                                      \
            INFO_DUMP("returned from catch_all");       \
            fprintf(stderr, " %s\n",                    \
                    STR(f));                            \
        }                                               \
        __catching_exit__ = 0;                          \
    } while (0)                                         \

#define TIMEOUT(d, f)                                   \
    do {                                                \
        signal(SIGALRM, sig_handler);                   \
        snprintf(__tname__, MAX_DESCRIPTOR, STR(f));    \
        __timeout__ = 1;                                \
        alarm(d);                                       \
        uerr = 0;                                       \
        if (!setjmp(__to_loc__))                        \
            (f);                                        \
        if (uerr){                                      \
            INFO_DUMP("returned from timeout");         \
            fprintf(stderr, " %s\n",                    \
                    STR(f));                            \
        }                                               \
        __timeout__ = 0;                                \
    } while (0)                                         \


#define RANDOM(min, max)                                       \
    {(min + rand() / (RAND_MAX / (max - min)))}                \


#define DINT(v, ...)                            \
    do {                                        \
        INFO_DUMP("int");                       \
        fprintf(stderr, STR(v)": %d", v);       \
        fprintf(stderr, "\n");                  \
    } while(0)


/* Functions */

void sig_handler(int signo);


#endif



