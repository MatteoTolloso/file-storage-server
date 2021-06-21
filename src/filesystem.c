
#include <myfilesystem.h>

int fs_request_manager(FileSystem_t * fs, int clientFd, int requestType){

    int retVal = 0; // messaggio di risposta da inviare al client (0 se è ok)

    switch (requestType){
        
        case OPEN_F:;
            
            int len, flags;  
            char path[MAX_PATH]; 
            
            if (readn(clientFd, &len, sizeof(int)) != sizeof(int)) return -1;
            if (readn(clientFd, &path, len) != len) return -1;
            if (readn(clientFd, &flags, sizeof(int)) != sizeof(int)) return -1;
            
            retVal = openFile_handler(fs, clientFd, path, flags);
            
            if (writen(clientFd, &retVal, sizeof(int)) != sizeof(int)) return -1;
    
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
            retVal = E_BAD_RQ;
            if (writen(clientFd, &retVal, sizeof(int)) != sizeof(int)){
                return -1;
            }

            fprintf(stderr, "inviato: %d\n", retVal);

        break;

    }

    printFs(fs);
    return 0;

}

int openFile_handler(FileSystem_t * fs, int clientFd, char * path, int flags){
    
    EXIT_ON(pthread_mutex_lock(&fs->fs_lock), != 0);    // prendo lock fs

    File_t * file = searchFile(fs, path); // cerco il file 
    
    // agisco in base ai flag

    if(file == NULL){     // file non esisteva
        
        if(flags == 0 || flags == 2){     //nessun flag o solo O_LOCK
            EXIT_ON(pthread_mutex_unlock(&fs->fs_lock), != 0);   
            return E_NOT_EX; 
        }

        // devo aggiungere un file
        
        if(fs->actNumFile + 1 == fs->maxNumFile){   // rimpiazzamento
            if( cacheEvict(fs) != 0){
            EXIT_ON(pthread_mutex_unlock(&fs->fs_lock), != 0);   
            return E_NO_SPACE;
            }
        }

        File_t * newfile  = init_File(path, clientFd); 
        
        if(flags == 3){  // O_CREATE | O_LOCK
            newfile->lockedBy = clientFd;
        }

        if(fs->actNumFile == 0){
            fs->lastFile = newfile;
            fs->firstFile = newfile;
        }
        else{
            (fs->lastFile)->next = newfile;
            newfile->prev = fs->lastFile;
            fs->lastFile = newfile;
        }
        fs->actNumFile++;
        if(fs->actNumFile > fs->maxRNumFile) fs->maxRNumFile = fs->actNumFile;
        EXIT_ON(pthread_mutex_unlock(&fs->fs_lock), != 0);   
        return 0;
    }

    if(file != NULL){ // esiste

        if (flags == 1 || flags == 3){    // O_CREATE oppure O_CREATE | O_LOCK
            EXIT_ON(pthread_mutex_unlock(&fs->fs_lock), != 0);
            return E_ALR_EX;   
        }

        if(flags == 0){ // nessun flag
            list_insert( &file->openedBy, clientFd);
            EXIT_ON(pthread_mutex_unlock(&fs->fs_lock), != 0);   
            return 0;
        }

        if (flags == 2){    // O_LOCK

            if(file->lockedBy == 0){
                file->lockedBy = clientFd;
                EXIT_ON(pthread_mutex_unlock(&fs->fs_lock), != 0);   
                return 0;
            }
            else{
                EXIT_ON(pthread_mutex_unlock(&fs->fs_lock), != 0);   
                return E_ALR_LK;
            }

        }


    }

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

File_t * searchFile(FileSystem_t * fs, char * path){ // da cambiare con la tabella hash

    File_t * tmp = fs->firstFile;

    while(tmp != NULL){
        if(strncmp(tmp->path, path, MAX_PATH) == 0){
            return tmp;
        }
        tmp = tmp->next;
    }

    return NULL;

}

FileSystem_t * init_FileSystem(int maxNumFile, int maxSize){
    
    FileSystem_t * fs;
    EXIT_ON(fs = malloc(sizeof(FileSystem_t)), == NULL);
    
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

File_t * init_File(char* path, int clientFd){
    
    File_t * file;
    EXIT_ON(file = malloc(sizeof(File_t)), == NULL);

    EXIT_ON( file->path = malloc(sizeof(char) * (strlen(path) +1)), == NULL);
    EXIT_ON( strcpy(file->path, path), == NULL);
    file->size = 0;
    file->cont = NULL;
    file->prev = NULL;
    file->next = NULL;
    file->openedBy = NULL;
    list_insert(&file->openedBy, clientFd);
    EXIT_ON(pthread_mutex_init(&file->f_mutex, NULL), != 0);
    EXIT_ON(pthread_mutex_init(&file->f_order, NULL), != 0);
    EXIT_ON(pthread_cond_init(&file->f_go, NULL), != 0);
    file->f_activeReaders = 0;
    file -> f_activeWriters = 0;

    return file;
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

void f_doneWrite(File_t * file){
    EXIT_ON(pthread_mutex_lock(&file->f_mutex), != 0);
    file->f_activeWriters--;
    EXIT_ON( pthread_cond_signal(&file->f_go), != 0);
    EXIT_ON( pthread_mutex_unlock(&file->f_mutex), != 0);
}

int readn(int fd, void *ptr, size_t n) {  
    size_t   nleft;
    ssize_t  nread;

    nleft = n;
    while (nleft > 0) {
        if((nread = read(fd, ptr, nleft)) < 0) {
        if (nleft == n) return -1; /* error, return -1 */
        else break; /* error, return amount read so far */
    } else if (nread == 0) break; /* EOF */
    
    nleft -= nread;
    ptr   = (void*)((char*)ptr + nread);
    }
    return(n - nleft); /* return >= 0 */
}

int writen(int fd, void *ptr, size_t n) {  
    size_t   nleft;
    ssize_t  nwritten;

    nleft = n;
    while (nleft > 0) {
        if((nwritten = write(fd, ptr, nleft)) < 0) {
        if (nleft == n) return -1; /* error, return -1 */
        else break; /* error, return amount written so far */
        } else if (nwritten == 0) break; 
        nleft -= nwritten;
        ptr   = (void*)((char*)ptr + nwritten);
    }
    return(n - nleft); /* return >= 0 */
}


void printFs(FileSystem_t * fs){
    File_t * f = fs->firstFile;

    while(f != NULL){
        printf("path file: %s\n", f->path);
        f = f->next;
    }
}