
#include <parser.h>
#include <util.h>
#include <sharedqueue.h>
#include <pthread.h>
#include <stdlib.h>



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

    
    


    /* INIZIO generazione dei thread worker */

    // .......

    /* FINE generazione dei thread worker */



    /* INIZIO operazioni di chiusura */

    free(sck_name);
    free(ready_clients->set);
    pthread_mutex_destroy(&ready_clients->lock);
    pthread_cond_destroy(&ready_clients->full);
    pthread_cond_destroy(&ready_clients->empty);
    free(ready_clients);

    /* FINE generazione dei thread worker */




    return 0;

}
