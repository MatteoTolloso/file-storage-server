

#ifndef _UTIL_H
#define _UTIL_H

#define EXIT_ON( arg1, arg2) \
    if ((arg1) arg2){    \
        perror(#arg1);   \
        exit(-1);   \
    }



#endif /*_util.h*/