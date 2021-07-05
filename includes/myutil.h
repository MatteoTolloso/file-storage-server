

#ifndef _MY_UTIL_H
#define _MY_UTIL_H
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>

extern FILE * logFile;
void server_log(char * format, ... );

#define EXIT_ON( arg1, arg2) \
    if ((arg1) arg2){    \
        perror(#arg1);   \
        exit(-1);   \
    }
    
#endif /*_MY_UTIL_H*/