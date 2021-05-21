
#include <myparser.h>
#include <myutil.h>
#include <mysharedqueue.h>
#include <myconnection.h>
#include <myhandler.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/select.h>

/*
void * worker(SharedQueue_t * q){ // verificare le chiamate pthread
    int clientFd;
    clientFd = SharedQueue_pop(q);
    //printf("ho letto dalla coda l'fd %d\n", clientFd);
    sleep(2);

    write(pipeWriting_fd, ); // ritornare alla pipe l'opposto del client quando chiude

}
*/

int main(int argc, char ** argv){

    /* INIZIO installazione gestore segnali */
    
    handler_installer();

    /* FINE installazione gestore segnali */



    /* INIZIO configurazione dei parametri dal file config.txt */

    if (argc < 2) EXIT_ON("errore parametri",);
    
    char * sck_name; int max_num_file, max_dim_storage, num_thread_worker;

    parse(argv[1], &sck_name,  &max_num_file,  &max_dim_storage,  &num_thread_worker);

    printf("socket name:%s\nmax_num_file:%d\nmax_dim_storage:%d\nnum_thread_worker:%d\n", 
            sck_name, max_num_file, max_dim_storage, num_thread_worker);
    
    /* FINE configurazione dei parametri dal file config.txt */
    
    
    
    /* INIZIO preparazione della coda concorrente e delle pipe */

    SharedQueue_t * ready_clients = init_SharedQueue();

    int pipefd[2], pipeReadig_fd, pipeWriting_fd;
    int pipefd2[2], pipeSigReading;

    EXIT_ON( pipe(pipefd), != 0);
    EXIT_ON( pipe(pipefd2), != 0);

    pipeReadig_fd = pipefd[0];
    pipeWriting_fd = pipefd[1];
    pipeSigReading = pipefd2[0];
    pipeSigWriting = pipefd2[1];

    /* FINE preaprazione della coda concorrente e delle pipe */


    /* INIZIO generazione dei thread worker */
    

    /* FINE generazione dei thread worker */


    /* INIZIO gestione delle richeste */
    
    int socket_fd = init_server(sck_name);
    int activeClients = 0;
    fd_set tmpset, set;
    FD_ZERO(&tmpset);
    FD_ZERO(&set);
    FD_SET(socket_fd, &set);
    FD_SET(pipeSigReading, &set);
    int fd_max = socket_fd; // attenzione
    int endMode=0;

    while(endMode==0 || activeClients > 0){  // finchè non è richiesta la terminazione o ci sono client attivi

        tmpset = set;
        if( select(fd_max + 1, &tmpset, NULL, NULL, NULL) == -1) // attenzione all'arrivo del segnale
        { 
            if(errno == EINTR) printf("un segnale ha interrotto la select\n");
            else EXIT_ON("errore sconosciuto",);
        }
         
        if(FD_ISSET(pipeSigReading, &tmpset)){  // è arrivato un segnale
            printf("fd lettore della pipe segnali settato\n");
            EXIT_ON(read(pipeSigReading, &endMode, sizeof(int)), != sizeof(int));
            
            printf("valore arrivato dalla pipe dei segnali: %d\n", endMode);

            if(endMode == 1) FD_CLR(socket_fd, &set);   // terminazione lenta, non ascolto più il socket
                  
            if(endMode == 2){   // terminazione veloce
                FD_ZERO(&set);
                FD_SET(pipeReadig_fd, &set);    
                // ascolto solo la pipe che torna i fd per essere sicuro che tutte le richieste in corso terminino

                for(int i=0; i<fd_max+1; i++){  // chiudo i client connessi (ma che non hanno richieste in corso)
                    if(FD_ISSET(i, &set) && (i != pipeReadig_fd) && (i != socket_fd)) close(i);
                }
            }
            continue;
        }

        for (int i = 0; i< fd_max +1; i++){
            if(FD_ISSET(i, &tmpset)){
                
                if( i == socket_fd){     // è arrivata una nuova richesta di connessione
                    int newConnFd;
                    EXIT_ON(newConnFd = accept(socket_fd, NULL, NULL), == -1);
                    FD_SET(newConnFd, &set);
                    activeClients++;
                    if(newConnFd > fd_max) fd_max = newConnFd;
                }
                else if(i == pipeReadig_fd){  // fd di ritorno dalla pipe
                    int returnedConnFd;
                    EXIT_ON(read(i, &returnedConnFd, sizeof(int)), != sizeof(int));
                    
                    if ((endMode == 0) || (endMode == 1)){  // devo continuare a servire i client connessi
                        if(returnedConnFd < 0){ //il client ha chiuso
                            activeClients--;   
                            close(-1 * returnedConnFd);
                        }
                        else{   // il client ha terminato la richiesta correttamente
                            FD_SET(returnedConnFd, &set); // lo rimetto in ascolto
                        }
                    }
                    else{   // sto terminando velocemente
                        if(returnedConnFd < 0){ //il client ha chiuso
                            activeClients--;   
                            close(-1 * returnedConnFd);
                        }
                        else{   // il client ha terminato la richiesta correttamente
                            activeClients--;   
                            close(returnedConnFd);                        
                        }
                    } 
                }
                else{   // un client già conesso ha inviato una richiesta (oppure ha chiuso)
                    if (endMode != 2){
                        SharedQueue_push(ready_clients, i); // verrà gestito da un worker
                        FD_CLR(i, &set);
                        if (i == fd_max) fd_max = updatemax(set, fd_max);
                    }
                    else{
                        close(i);
                        activeClients--;
                    }
                }
            }
        }
    }

    /* FINE gestione delle richieste */



    /* INIZIO operazioni di chiusura */
    
    unlink(sck_name);
    free(sck_name);
    free(ready_clients->set);
    pthread_mutex_destroy(&ready_clients->lock);
    pthread_cond_destroy(&ready_clients->full);
    pthread_cond_destroy(&ready_clients->empty);
    free(ready_clients);
    close(pipeReadig_fd);
    close(pipeWriting_fd);

    /* FINE operazioni di chiusura */

    return 0;

}
