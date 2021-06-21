
#include <myfilesystem.h>

void fs_init(){
    // inizializza le variabili del fs
    // per ogni file inizializza le variabili

}

int fs_request_manager(Filesystem_t * fs, int clientFd, int requestType){

    int retVal = 0;

    switch (requestType){
        
        case OPEN_F:;
            
            int len, flags;  
            char path[MAX_PATH]; 
            
            if (read(clientFd, &len, sizeof(int)) != sizeof(int)){
                return -1;
            }
            if (read(clientFd, &path, len) != len){
                return -1;
            }
            if (read(clientFd, &flags, sizeof(int)) != sizeof(int)){
                return -1;
            }
            
            retVal = openFile_handler(fs, clientFd, path, flags);
            
            if (read(clientFd, &retVal, sizeof(int)) != sizeof(int)){
                return -1;
            }
            
            return 0; // successo
            

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

int openFile_handler(Filesystem_t * fs, int clientFd, char * path, int flags){
    

    EXIT_ON(pthread_mutex_lock(&fs->fs_lock), != 0);    // prendo lock fs

    File_t * file = searchFile(fs, path); // cerco il file 

    // agisco in base ai flag

    if(flags == 0){     // nessun flag
        return E_NOT_EX;
    }
    
    //rilascio lock

    return 0;
}
/*
int readFile_handler(Filesystem_t * fs, char * path, void * buf, size_t * size ){

    EXIT_ON(pthread_mutex_lock(&fs->fs_lock), != 0);    // lock del fs

    File_t * file = findFile(fs, path);     // funzione che assume di avere la lock

    f_startRead(file);  // voglio entrare in sezione critica da lettore

    EXIT_ON(pthread_mutex_unlock(&fs->fs_lock), != 0);    // rilascio lock del fs

    // ... operazioni di lettura, trasferisco il contenuto del file in buf e aggiorno size

    f_doneRead(file);   // esco dalla sezione critica

    return 1;

}

int writeFile_handler(Filesystem_t * fs, char * path, void * buf, size_t * size){

    EXIT_ON(pthread_mutex_lock(&fs->fs_lock), != 0);    // lock del fs

    File_t * file = findFile(fs, path);     // funzione che assume di avere la lock

    f_startWrite(file);  // voglio entrare in sezione critica da scrittore

    // ... operazioni di scrittura, con aggiornamento del file e del fs

    f_doneRead(file);   // esco dalla sezione critica

    EXIT_ON(pthread_mutex_unlock(&fs->fs_lock), != 0);    // rilascio lock del fs


    return 1;

}

int removeFile_handler(Filesystem_t * fs, char * path, void * buf, size_t * size){

    EXIT_ON(pthread_mutex_lock(&fs->fs_lock), != 0);    // lock del fs

    File_t * file = findFile(fs, path);     // funzione che assume di avere la lock

    f_startRemove(file);  // voglio entrare in sezione critica da scrittore sapendo che oltre ad essere solo non c'è nessuno in attesa

    // ... operazioni di eleminazione, con aggiornamento del file e del fs

    EXIT_ON(pthread_mutex_unlock(&fs->fs_lock), != 0);    // rilascio lock del fs


    return 1;

}
*/

File_t * searchFile(Filesystem_t * fs, char * path){ // da cambiare con la tabella hash

    File_t * tmp = fs->firstFile;

    while(tmp != NULL){
        if(strncmp(tmp->path, path, MAX_PATH) == 0){
            return tmp;
        }
        tmp = tmp->next;
    }

    return NULL;

}

Filesystem_t * init_FileSystem(int maxNumFile, int maxSize){
    
    Filesystem_t * fs;
    EXIT_ON(fs = malloc(sizeof(fs)), == NULL);
    
    fs->actSize = 0;
    fs->actNumFile = 0;
    fs->maxSize = maxSize;
    fs->maxNumFile = maxNumFile;
    fs->maxRSize = 0;
    fs->maxRNumFile = 0;
    fs->cacheAlgoCount = 0;

    EXIT_ON(pthread_mutex_init(&fs->fs_lock, NULL), != 0);

    fs->firstFile = NULL;
    fs->lastFile = NULL;

    return fs;
}

void f_init(File_t * file){
    file->path = NULL;
    file->size = 0;
    file->cont = NULL;
    file->prev = NULL;
    file->next = NULL;
    file->f_activeReaders = 0;
    file->f_activeWriters = 0;
    EXIT_ON(pthread_mutex_init(&file->f_mutex, NULL), != 0);
    EXIT_ON(pthread_mutex_init(&file->f_order, NULL), != 0);
    EXIT_ON(pthread_cond_init(&file->f_go, NULL), != 0);
}

void f_startRead(File_t * file){
    EXIT_ON(pthread_mutex_lock(&file->f_order), != 0); // sono in fila
    EXIT_ON(pthread_mutex_lock(&file->f_mutex), != 0); // sono in sezione critica
    while(file->f_activeWriters > 0){     // se ci sono scrittori attivi mi sospendo (ma mantengo il posto in fila)
        EXIT_ON(pthread_cond_wait(&file->f_go, &file->f_mutex), != 0 );
    }
    file->f_activeReaders++;
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

void f_startRemove(File_t * file){
    EXIT_ON(pthread_mutex_lock(&file->f_order), != 0); // sono in fila
    EXIT_ON(pthread_mutex_lock(&file->f_mutex), != 0 ); // sono in sezione critica
    while(file->f_activeReaders > 0 || file->f_activeWriters > 0){     // se ci sono scrittori o lettori attivi mi sospendo (ma mantenfo il posto in fila)
        EXIT_ON(pthread_cond_wait(&file->f_go, &file->f_mutex), != 0 );
    }
    EXIT_ON(pthread_mutex_unlock(&file->f_order), != 0); // rilascio il posto in fila
    EXIT_ON(pthread_mutex_unlock(&file->f_mutex), != 0); // rilascio la mutex
}



void f_doneWrite(File_t * file){
    EXIT_ON(pthread_mutex_lock(&file->f_mutex), != 0);
    file->f_activeWriters--;
    EXIT_ON( pthread_cond_signal(&file->f_go), != 0);
    EXIT_ON( pthread_mutex_unlock(&file->f_mutex), != 0);
}


