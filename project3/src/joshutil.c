/******************************************************************************/
/*                                                                            */
/* FILE    : util.c                                                           */
/* AUTHOR  : Joshua Miller                                                    */
/* PROJECT : Project 2                                                        */
/* CLASS   : Parallel Computing - Winter 2014                                 */
/*                                                                            */
/******************************************************************************/

#include <signal.h>
#include <pthread.h>

#include "joshutil.h"

int __timeout__           = 0;
int __catching_exit__     = 0;
int __catching_segfault__ = 0;

int uerr = 0;

/* Obtain a backtrace and print it to stdout. */
void print_trace (void)
{
    void *array[10];
    size_t size;
    char **strings;
    size_t i;
     
    size = backtrace (array, 10);
    strings = backtrace_symbols (array, size);
     
    printf ("Obtained %zd stack frames.\n", size);
     
    for (i = 0; i < size; i++)
        printf ("%s\n", strings[i]);
     
    free (strings);
}

void sig_handler(int signo)
{
    if (signo == SIGSEGV){
        fprintf(stderr, "%sSEGFAULT%s: %s: printing stack frame ...\n",
                __red__, __nrm__, __fname__);
        print_trace();
        if (__catching_segfault__){
            fprintf(stderr, "\treturning to calling function prior to SEGFAULT ... \n");
            uerr = SIGSEGV;
            longjmp(__seg_loc__, 1); 
        } else {
            exit(SIGSEGV);
        }
    }
    if (signo == SIGALRM){
        if (__timeout__){
            fprintf(stderr, "%sTIMEOUT%s: %s\n", __red__, __nrm__, __tname__);
            uerr = SIGALRM;
            longjmp(__to_loc__, 1); 
        }
    }
}



