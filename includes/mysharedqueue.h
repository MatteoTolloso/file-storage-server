
#ifndef _MY_SHARED_QUEUE_H
#define _MY_SHARED_QUEUE_H

#include <myutil.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define SHARED_QUEUE_MAX_DIM 2048

typedef struct _sq{
    int *set;
    int start;
    int current_size;
    pthread_mutex_t lock;
    pthread_cond_t full;
    pthread_cond_t empty;
}SharedQueue_t;

SharedQueue_t * init_SharedQueue();

void SharedQueue_push(SharedQueue_t * q, int fd_c);

int SharedQueue_pop(SharedQueue_t * q);

#endif /*_MY_SHARED_QUEUE_H */