

#ifndef _UTIL_H
#define _UTIL_H
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#define EXIT_ON( arg1, arg2) \
    if ((arg1) arg2){    \
        perror(#arg1);   \
        exit(-1);   \
    }



#endif /*_util.h*/