
#include <parser.h>
#include <util.h>
#include <sharedqueue.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/select.h>
#include <connection.h>



int main(int argc, char ** argv){

    /* INIZIO configurazione dei parametri dal file config.txt */
    
    char * sck_name; int max_num_file, max_dim_storage, num_thread_worker;

    

    parse(argv[1], &sck_name,  &max_num_file,  &max_dim_storage,  &num_thread_worker);

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
    
    int socket_fd = init_server(sck_name);

    fd_set tmpset, set;
    FD_ZERO(&tmpset);
    FD_ZERO(&set);
    FD_SET(socket_fd, &set);
    int fd_max = socket_fd; // attenzione

   
    while(1){
        tmpset = set;
        EXIT_ON(select(fd_max + 1, &tmpset, NULL, NULL, NULL), == -1); // attenzione all'arrivo del segnale 
                
        for (int i = 0; i< fd_max +1; i++){
            if(FD_ISSET(i, &tmpset)){
                
                if(i == socket_fd){     // è arrivata una nuova richesta di connessione
                    int newConnFd;
                    EXIT_ON(newConnFd = accept(socket_fd, NULL, NULL), == -1);
                    FD_SET(newConnFd, &set);
                    if(newConnFd > fd_max) fd_max = newConnFd;
                    continue;
                }
                else if(i == pipeReadig_fd){  // fd di rotorno dalla pipe
                    int returnedConnFd;
                    EXIT_ON(read(i, &returnedConnFd, sizeof(int)), != sizeof(int));
                    FD_SET(returnedConnFd, &set);
                    continue;
                }
                else{   // un client già conesso ha inviato un messaggio
                    SharedQueue_push(ready_clients, i); // verrà gestito da un worker
                    FD_CLR(i, &set);
                    if (i == fd_max) fd_max = updatemax(set, fd_max);
                    continue;
                }
            }
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
