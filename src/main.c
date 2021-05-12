
#include <parser.h>
#include <util.h>

#include <pthread.h>

#define SHARED_QUEUE_MAX_DIM 16

typedef struct _sq{
    int *set;
    int start;
    int current_size;
    pthread_mutex_t lock;
    pthread_cond_t full;
    pthread_cond_t empty;
}SharedQueue_t;

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




int main(void){

    /* INIZIO configurazione dei parametri dal file config.txt */
    char * sck_name; int max_num_file, max_dim_storage, num_thread_worker;

    EXIT_ON(parse(&sck_name,  &max_num_file,  &max_dim_storage,  &num_thread_worker), != 0);

    printf("socket name:%s\nmax_num_file:%d\nmax_dim_storage:%d\nnum_thread_worker:%d\n", 
            sck_name, max_num_file, max_dim_storage, num_thread_worker);
    
    /* FINE configurazione dei parametri dal file config.txt */
    
    
    
    /* INIZIO preparazione della coda concorrente e della pipe */

    SharedQueue_t * ready_clients = init_SharedQueue();

    /* FINE preaprazione della coda concorrente e della pipe */


    SharedQueue_push(ready_clients, 2);
    SharedQueue_push(ready_clients, 5);


    printf("%d\n", SharedQueue_pop(ready_clients));
    printf("%d\n", SharedQueue_pop(ready_clients));
    printf("%d\n", SharedQueue_pop(ready_clients));
    printf("%d\n", SharedQueue_pop(ready_clients));





    /* INIZIO generazione dei thread worker */

    // .......

    /* FINE generazione dei thread worker */






    return 0;

}
