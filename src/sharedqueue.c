#include <sharedqueue.h>
#include <util.h>
#include <errno.h>

SharedQueue_t * init_SharedQueue(){

    SharedQueue_t * q;

    EXIT_ON( q = (SharedQueue_t*)malloc(sizeof(SharedQueue_t)), == NULL);

    EXIT_ON( memset(q, '0', sizeof(SharedQueue_t)), == NULL);

    EXIT_ON(q->set = malloc(sizeof(int) * SHARED_QUEUE_MAX_DIM), == NULL);

    EXIT_ON(pthread_mutex_init(&q->lock, NULL), != 0);

    EXIT_ON(pthread_cond_init(&q->full, NULL), != 0);

    EXIT_ON(pthread_cond_init(&q->empty, NULL), != 0);

    q->start = 0;

    q->current_size = 0;

    return q;

}

void SharedQueue_push(SharedQueue_t * q, int fd_c){
    EXIT_ON(q, == NULL);

    EXIT_ON(pthread_mutex_lock(&q->lock), != 0);

    while(q->current_size >= SHARED_QUEUE_MAX_DIM){
        EXIT_ON(pthread_cond_wait(&q->full, &q->lock), != 0);
    }

    q->set[ (q->start + q->current_size) % SHARED_QUEUE_MAX_DIM] = fd_c;

    q->current_size++;

    EXIT_ON(pthread_cond_signal(&q->empty), != 0);

    EXIT_ON(pthread_mutex_unlock(&q->lock), != 0);
}

int SharedQueue_pop(SharedQueue_t * q){
    EXIT_ON(q, == NULL);

    EXIT_ON(pthread_mutex_lock(&q->lock), != 0);

    while(q->current_size <= 0){
        EXIT_ON(pthread_cond_wait(&q->empty, &q->lock), != 0);
    }

    int ret = q->set[ q->start % SHARED_QUEUE_MAX_DIM];

    q->start++;

    q->current_size--;

    q->start = q->start % SHARED_QUEUE_MAX_DIM;

    EXIT_ON(pthread_cond_signal(&q->full), != 0);

    EXIT_ON(pthread_mutex_unlock(&q->lock), != 0);

    return ret;
}