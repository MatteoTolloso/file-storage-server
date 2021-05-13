
#include <parser.h>
#include <util.h>
#include <sharedqueue.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/select.h>
// da mettere nell'header di inizializzazione del socket
#include <sys/socket.h>
#include <sys/un.h>
#define UNIX_PATH_MAX 108
#define MAX_CONNECTION_QUEUE 16


int main(void){

    /* INIZIO configurazione dei parametri dal file config.txt */
    
    char * sck_name; int max_num_file, max_dim_storage, num_thread_worker;

    parse(&sck_name,  &max_num_file,  &max_dim_storage,  &num_thread_worker);

    printf("socket name:%s\nmax_num_file:%d\nmax_dim_storage:%d\nnum_thread_worker:%d\n", 
            sck_name, max_num_file, max_dim_storage, num_thread_worker);
    
    /* FINE configurazione dei parametri dal file config.txt */
    
    
    
    /* INIZIO preparazione della coda concorrente e della pipe */

    SharedQueue_t * ready_clients = init_SharedQueue();

    int pipefd[2], pipeReadig_fd, pipeWriting_fd;

    EXIT_ON( pipe(pipefd), != 0);

    pipeReadig_fd = pipefd[0];
    pipeWriting_fd = pipefd[1];

    /* FINE preaprazione della coda concorrente e della pipe */


    /* INIZIO generazione dei thread worker */
    // ricorda di mascherare tutti i segnali
    // .......

    /* FINE generazione dei thread worker */

    /* INIZIO gestione delle richeste */

    // prima parte da mettere in un file separato
    EXIT_ON(strlen(sck_name) >= UNIX_PATH_MAX, != 0);
    int socket_fd;
    EXIT_ON(socket_fd = socket(AF_UNIX, SOCK_STREAM, 0), == -1);
    struct sockaddr_un server_addr;
    EXIT_ON(memset(&server_addr, 0, sizeof(struct sockaddr_un)), == NULL);
    server_addr.sun_family = AF_UNIX;
    EXIT_ON(strncpy(server_addr.sun_path, sck_name, strlen(sck_name) + 1),  == NULL);
    EXIT_ON(bind(socket_fd, (struct sockaddr_un *)&server_addr, sizeof(struct sockaddr_un)), != 0);
    EXIT_ON(listen(socket_fd, MAX_CONNECTION_QUEUE), != 0);

    int nread=0;
    fd_set tmpset, set;
    FD_ZERO(&tmpset);
    FD_ZERO(&set);
    FD_SET(socket_fd, &set);
    int fd_max = socket_fd; // attenzione

    // aggiungi pipe per la gestione dei segnali
    while(1){
        tmpset = set;
        EXIT_ON(select(fd_max + 1, &tmpset, NULL, NULL, NULL), == -1); // attenzione all'arrivo del segnale 
                // idea thread che si occupa dei segnali e che manda informazioni tramite la pipe

        //if( FD_ISSET(signal_pipe_read, &tmpset)){ }// è arrivato un segnale


        

        for(int i=0; i< fd_max + 1, i++){

            


        }


    }





    /* FINE gestione delle richieste */



    /* INIZIO operazioni di chiusura */

    free(sck_name);
    free(ready_clients->set);
    pthread_mutex_destroy(&ready_clients->lock);
    pthread_cond_destroy(&ready_clients->full);
    pthread_cond_destroy(&ready_clients->empty);
    free(ready_clients);
    close(pipeReadig_fd);
    close(pipeWriting_fd);

    /* FINE generazione dei thread worker */




    return 0;

}
