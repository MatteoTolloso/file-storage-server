
#include <myparser.h>
#include <myutil.h>
#include <mysharedqueue.h>
#include <myconnection.h>
#include <myhandler.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/time.h>
#include <myfilesystem.h>

int pipeSigWriting; // from myhandler.h
FILE * logFile; // from myutil.h

void server_log(char * format, ... );
char * flgToStr(int flg);

typedef struct _worker_args{
    SharedQueue_t * q;
    int pipeWriting_fd;
    FileSystem_t * fs;
}WorkerArgs;


void * worker(void * args){
    
    worker_handler_installer();

    SharedQueue_t * q = ((WorkerArgs *)(args))->q;
    int pipeWriting_fd = ((WorkerArgs *)(args))->pipeWriting_fd;
    FileSystem_t * fs = ((WorkerArgs *)(args))->fs;
    
    while(1){
        
        int clientFd = SharedQueue_pop(q); // preleva un client dalla coda condivisa
        
        if(clientFd == -1) return 0; // il manager mi dice di terminare

        int requestType;

        if(read(clientFd, &requestType, sizeof(int)) <= 0 ){ // leggi la richesta oppure il client ha chiuso
            EXIT_ON(fs_request_manager(fs, clientFd, CLOSE_ALL), != 0);
            clientFd*=-1;
            EXIT_ON(write(pipeWriting_fd, &clientFd, sizeof(int)), != sizeof(int)); // il manager si occuperà di chiuderlo per evitare racecondition con i fd
            continue;
        }

        int result = fs_request_manager(fs, clientFd, requestType);
        
        // se è 0 vuol dire che ho terminato correttamente la richiesta e devo dire al manager di rimettermi in ascolto di quel fd
        // altrimenti vuol dire che il client ha chiuso inaspettatamente durante le comunicazioni e devo dire al manager di chiudere il fd
        
        if (result == 0){
            EXIT_ON(write(pipeWriting_fd, &clientFd, sizeof(int)), != sizeof(int));
        }
        else{
            EXIT_ON(fs_request_manager(fs, clientFd, CLOSE_ALL), != 0);
            clientFd*=-1; // il client ha avuto problemi e lo chiudo
            EXIT_ON(write(pipeWriting_fd, &clientFd, sizeof(int)), != sizeof(int));
        }
    }
}


int main(int argc, char ** argv){

    /* INIZIO installazione gestore segnali */
    
    handler_installer();

    /* FINE installazione gestore segnali */

    remove("mysock");

    /* INIZIO configurazione dei parametri dal file config.txt e del file di log */

    if (argc < 2) EXIT_ON("errore parametri",);
    
    char * sck_name, *log_path; int max_num_file, max_dim_storage, num_thread_worker;

    parse(argv[1], &sck_name,&log_path,  &max_num_file,  &max_dim_storage,  &num_thread_worker);

    EXIT_ON(logFile = fopen(log_path, "w"), == NULL);

    printf("MANAGER:\nsocket name:%s\nlog_path:%s\nmax_num_file:%d\nmax_dim_storage:%d\nnum_thread_worker:%d\n", 
            sck_name,log_path, max_num_file, max_dim_storage, num_thread_worker);
    
    /* FINE configurazione dei parametri dal file config.txt e del file di log */
    
    
    
    /* INIZIO preparazione della coda concorrente, della pipe e del fs */

    SharedQueue_t * ready_clients = init_SharedQueue();

    int pipefd[2], pipeReadig_fd, pipeWriting_fd;
    int pipefd2[2], pipeSigReading;

    EXIT_ON( pipe(pipefd), != 0);
    EXIT_ON( pipe(pipefd2), != 0);

    pipeReadig_fd = pipefd[0];
    pipeWriting_fd = pipefd[1];
    pipeSigReading = pipefd2[0];
    pipeSigWriting = pipefd2[1];    // dichiarata come globale in myhandler.h

    FileSystem_t * fs = init_FileSystem(max_num_file, max_dim_storage);

    /* FINE preaprazione della coda concorrente,della pipe e del fs */


    /* INIZIO generazione dei thread worker */
    
    pthread_t * tidArr;
    EXIT_ON( tidArr = malloc(sizeof(pthread_t) * num_thread_worker), == NULL);
    WorkerArgs * args;
    EXIT_ON(args = malloc(sizeof(WorkerArgs)), == NULL);
    
    args->pipeWriting_fd = pipeWriting_fd;
    args->q = ready_clients;
    args->fs = fs;

    for(int i=0; i<num_thread_worker; ++i){
	    EXIT_ON(pthread_create(&tidArr[i], NULL, worker, (void*)args), != 0);
    }


    /* FINE generazione dei thread worker */


    /* INIZIO gestione delle richeste */
    
    int socket_fd = init_server(sck_name);
    int activeClients = 0;
    fd_set tmpset, set;
    FD_ZERO(&tmpset);
    FD_ZERO(&set);
    FD_SET(socket_fd, &set);
    FD_SET(pipeSigReading, &set);
    FD_SET(pipeReadig_fd, &set);
    int fd_max = find_max(socket_fd, pipeReadig_fd, pipeWriting_fd, pipeSigReading, pipeSigWriting);
    fd_max++; // per sicurezza, perchè c'è anche da considerare il fd del file config
    int endMode=0;

    while(endMode==0 || activeClients > 0){  // finchè non è richiesta la terminazione o ci sono client attivi
        //fprintf(stderr, "\nendMode = %d, activeClients = %d, fd_max = %d\n", endMode, activeClients, fd_max);
        tmpset = set;        
        if( select(fd_max + 1, &tmpset, NULL, NULL, NULL) == -1) // attenzione all'arrivo del segnale
        { 
            if(errno == EINTR) server_log("Segnale ricevuto");
            else EXIT_ON("errore sconosciuto",);
        }
         
        if(FD_ISSET(pipeSigReading, &tmpset)){  // è arrivato un segnale
            EXIT_ON(read(pipeSigReading, &endMode, sizeof(int)), != sizeof(int));
            
            if(endMode == 1) {
                FD_CLR(socket_fd, &set);   // terminazione lenta, non ascolto più il socket
                updatemax(set, fd_max);
                server_log("inizio terminazione lenta");
            }

            if(endMode == 2){   // terminazione veloce 
                
                for(int i=0; i<fd_max+1; i++){  // chiudo i client connessi (ma che non hanno richieste in corso)
                    if(FD_ISSET(i, &set) && (i != pipeReadig_fd) && (i != socket_fd)) close(i);
                }
                FD_CLR(socket_fd, &set); // non ascolto nuove richieste di connessione
                fd_max = updatemax(set, fd_max);
                server_log("inizio terminazione veloce");
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
                    server_log("nuova connessione con client %d", newConnFd);
                }
                else if(i == pipeReadig_fd){  // fd di ritorno dalla pipe
                    int returnedConnFd;
                    EXIT_ON(read(pipeReadig_fd, &returnedConnFd, sizeof(int)), != sizeof(int));
                    
                    if ((endMode == 0) || (endMode == 1)){  // devo continuare a servire i client connessi
                        if(returnedConnFd < 0){ //il client ha chiuso
                            activeClients--;
                            returnedConnFd*=-1;   
                            close(returnedConnFd);
                        }
                        else{   // il client ha terminato la richiesta correttamente
                            FD_SET(returnedConnFd, &set); // lo rimetto in ascolto
                            if(returnedConnFd > fd_max) fd_max = returnedConnFd;
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
                        FD_CLR(i, &tmpset);
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

    server_log("Uscito dal ciclo manager");
    /* FINE gestione delle richieste */



    /* INIZIO chiusura thread */

    for (int i=0; i<num_thread_worker; i++){
        SharedQueue_push(ready_clients, -1);
    }

    for (int i= 0; i< num_thread_worker; i++){
        EXIT_ON(pthread_join(tidArr[i], NULL), == -1);
    }

    
    /* FINE chiusura thread */
    
    

    /* INIZIO operazioni di chiusura */

    // dealloca il fs con tutti i file
        
    deinit_FileSystem(fs);
    close(pipeReadig_fd);
    close(pipeWriting_fd);
    close(pipeSigReading);
    close(pipeSigWriting);
    remove(sck_name);
    free(sck_name);
    fclose(logFile);
    free(log_path);
    free(tidArr);
    free(args);
    
    free(ready_clients->set);
    pthread_mutex_destroy(&ready_clients->lock);
    pthread_cond_destroy(&ready_clients->full);
    pthread_cond_destroy(&ready_clients->empty);
    free(ready_clients);
    
    

    /* FINE operazioni di chiusura */

    return 0;

}

void server_log(char * format, ... ){
    va_list arglist;
    va_start(arglist, format);
    vfprintf(logFile, format, arglist);
    fprintf(logFile, "\n");
    fflush(logFile);
    va_end(arglist);
}
