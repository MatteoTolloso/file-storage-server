
#include <myfilesystem.h>

void fs_init(){
    // inizializza le variabili del fs
    // per ogni file inizializza le variabili

}

int fs_request_manager(int clientFd, int requestType){

    int retVal = 0;

    switch (requestType){
        case OPEN_F:

        break;

        case READ_F:

        break;

        case READ_N_F:

        break;

        case WRITE_F:

        break;

        case APPEND_T_F:

        break;

        case LOCK_F:

        break;

        case UNLOCK_F:

        break;

        case CLOSE_F:

        break;

        case REMOVE_F:

        break;

        default:
            fprintf(stderr, "Richesta non disponibile\n");
            retVal = 1;
        break;

    }

    return retVal;

}

void f_init(File_t * file){
    file->path = NULL;
    file->size = 0;
    file->cont = NULL;
    file->prev = NULL;
    file->next = NULL;
    file->f_activeReaders = 0;
    file->f_activeWriters = 0;
    file->f_queueSize = 0;
    EXIT_ON(pthread_mutex_init(&file->f_mutex, NULL), != 0);
    EXIT_ON(pthread_mutex_init(&file->f_order, NULL), != 0);
    EXIT_ON(pthread_cond_init(&file->f_go, NULL), != 0);
}

void f_startRead(File_t * file){
    EXIT_ON(pthread_mutex_lock(&file->f_order), != 0); // sono in fila
    EXIT_ON(pthread_mutex_lock(&file->f_mutex), != 0 ); // sono in sezione critica
    while(file->f_activeWriters > 0){     // se ci sono scrittori attivi mi sospendo (ma mantengo il posto in fila)
        EXIT_ON(pthread_cond_wait(&file->f_go, &file->f_mutex), != 0 );
    }
    file->f_activeReaders++;
    file->f_queueSize--;
    EXIT_ON(pthread_mutex_unlock(&file->f_order), != 0); // rilascio il posto in fila
    EXIT_ON(pthread_mutex_unlock(&file->f_mutex), != 0); // rilascio la mutex
}

void f_doneRead(File_t * file){
    EXIT_ON(pthread_mutex_lock(&file->f_mutex), != 0);
    file->f_activeReaders--;
    if(file->f_activeReaders == 0){
        EXIT_ON( pthread_cond_signal(&file->f_go), != 0);   // faccio entrare il prossimo in fila (se ci sono lettori attivi è inutile)
    }
    EXIT_ON( pthread_mutex_unlock(&file->f_mutex), != 0);
}

void f_startWrite(File_t * file){
    EXIT_ON(pthread_mutex_lock(&file->f_order), != 0); // sono in fila
    EXIT_ON(pthread_mutex_lock(&file->f_mutex), != 0 ); // sono in sezione critica
    while(file->f_activeReaders > 0 || file->f_activeWriters > 0){     // se ci sono scrittori o lettori attivi mi sospendo (ma mantenfo il posto in fila)
        EXIT_ON(pthread_cond_wait(&file->f_go, &file->f_mutex), != 0 );
    }
    file->f_activeWriters++;
    EXIT_ON(pthread_mutex_unlock(&file->f_order), != 0); // rilascio il posto in fila
    EXIT_ON(pthread_mutex_unlock(&file->f_mutex), != 0); // rilascio la mutex
}

void f_doneWrite(File_t * file){
    EXIT_ON(pthread_mutex_lock(&file->f_mutex), != 0);
    file->f_activeWriters--;
    EXIT_ON( pthread_cond_signal(&file->f_go), != 0);
    EXIT_ON( pthread_mutex_unlock(&file->f_mutex), != 0);
}


