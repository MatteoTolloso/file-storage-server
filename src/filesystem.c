
#include <myfilesystem.h>

int fs_request_manager(FileSystem_t * fs, int clientFd, int requestType){

    int retVal = 0; // messaggio di risposta da inviare al client (0 se è ok)

    switch (requestType){
        
        case OPEN_F:;{
            
            int flags;  
            char path[MAX_PATH]; 
            
            if (readn(clientFd, &path, MAX_PATH) != MAX_PATH) return -1;
            if (readn(clientFd, &flags, sizeof(int)) != sizeof(int)) return -1;
            fprintf(stderr, "open file, path: %s, flags: %d\n", path, flags );

            retVal = openFile_handler(fs, clientFd, path, flags);
            
            if (writen(clientFd, &retVal, sizeof(int)) != sizeof(int)) return -1;
        }
        break;

        case READ_F:

        break;

        case READ_N_F:

        break;

        case WRITE_F:;{
            int len, retBufLen;
            char path[MAX_PATH];
            char *buf, *retBuf = NULL;
            if (readn(clientFd, &path, MAX_PATH) != MAX_PATH) return -1;  // path
            if (readn(clientFd, &len, sizeof(int)) != sizeof(int)) return -1; // lunghezza file
            fprintf(stderr, "write file, path: %s, len: %d\n", path, len );
            EXIT_ON(buf = malloc(len), == NULL);
            if (readn(clientFd, buf, len) != len){free(buf); return -1;} // file

            //scrive nel fs il file con path e contenuto buf, mette in retFile i file da rimandare al client
            retVal = writeFile_handler(fs, clientFd, path, len, buf, &retBuf, &retBufLen);

            if (writen(clientFd, &retVal, sizeof(int)) != sizeof(int)) return -1;
            
            if(retVal > 1) free(buf);   // c'è stato un errore interno nella richiesta
            if (retVal == 1){       // sono stati espulsi file
                if (writen(clientFd, &retBuf, retBufLen) != retBufLen) {
                    free(retBuf); 
                    return -1;
                }
                else{
                    free(retBuf);
                }
            }
        }
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

int writeFile_handler(FileSystem_t * fs, int clientFd, char* path, int len, char * buf, char ** retBuf, int * retBufLen){

    EXIT_ON(pthread_mutex_lock(&fs->fs_lock), != 0);    // prendo lock fs
    

    File_t * file = searchFile(fs, path); // cerco il file con quel path
    

    if(file == NULL){
        EXIT_ON(pthread_mutex_unlock(&fs->fs_lock), != 0);
        return E_NOT_EX;
    }

    f_startRead(file); // devo modificare il file, entro il scrittura
    

    if(!list_mem(&file->openedBy, clientFd)){    // controllo se ho aperto il file
        f_doneRead(file);
        EXIT_ON(pthread_mutex_unlock(&fs->fs_lock), != 0);
        return E_NOT_OPN;
    }
    /* temporaneamente non mi interesso alla lock
    if(clientFd != file->lockedBy ){    // controllo se ho la lock sul file
        f_doneRead(file);
        EXIT_ON(pthread_mutex_unlock(&fs->fs_lock), != 0);
        return E_LOCK;
    }
    */
    if(file->size != 0){        // controllo se il file è stato già scritto
        f_doneRead(file);
        EXIT_ON(pthread_mutex_unlock(&fs->fs_lock), != 0);
        return E_BAD_RQ;
    }
    
    if(fs->maxSize < len){  // non riuscirò mai a trovare lo spazio necessario
        f_doneRead(file);
        EXIT_ON(pthread_mutex_unlock(&fs->fs_lock), != 0);
        return E_NO_SPACE;
    }
    
    f_doneRead(file);

    // fase di evict e salvataggio dei file da mandare al client
    fprintf(stderr, "check1: %d %d\n", fs->actSize, len);
    int i_retBufLen = 0;
    char * i_retBuf = NULL;
    int endval = -1;
    while((fs->actSize + len > fs->maxSize)){
        File_t * tmp = cacheEvict(fs);  // scollega il file dal fs e non ci sono scrittori o lettori atttivi o in attesa
        fprintf(stderr, "after evict");        
        if(tmp == NULL){
            EXIT_ON(pthread_mutex_unlock(&fs->fs_lock), != 0);
            return E_NO_SPACE;
        }
        // devo inserire il file del buffer da mandare al client
        
        EXIT_ON(i_retBuf= realloc(i_retBuf, i_retBufLen + MAX_PATH + tmp->size + sizeof(int)), == NULL);
        memcpy(i_retBuf + i_retBufLen, tmp->path, MAX_PATH);   // path
        i_retBufLen += MAX_PATH;
        memcpy(i_retBuf + i_retBufLen, &file->size, sizeof(int)); // lunghezza
        i_retBufLen += sizeof(int);
        memcpy(i_retBuf + i_retBufLen, tmp->cont, tmp->size);    // contenuto
        i_retBufLen += tmp->size;
        fprintf(stderr, "buffer preparato\n");
        deleteFile(tmp);
    }
    if(i_retBuf != NULL){
        EXIT_ON(i_retBuf= realloc(i_retBuf, i_retBufLen + sizeof(int)), == NULL);
        memcpy(retBuf, &endval, sizeof(int));
        *retBuf = i_retBuf;
        *retBufLen = i_retBufLen;
    }
    fprintf(stderr, "check2: %d\n", *retBufLen);

    // scrittura nel file 

    f_startWrite(file);
    file->cont = buf;
    file->size = len;
    fs->actSize += len;
    if(fs->actSize >fs->maxRSize) fs->maxRSize = fs->actSize;
    f_doneWrite(file);
    EXIT_ON(pthread_mutex_unlock(&fs->fs_lock), != 0);
    

    if(*retBuf == NULL) return 0;
    else return 1;

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
            File_t * tmp = cacheEvict(fs);
            if( tmp == NULL){    // non riesco a rimpiazzare
            EXIT_ON(pthread_mutex_unlock(&fs->fs_lock), != 0);   
            return E_NO_SPACE;
            }
            else{
                deleteFile(tmp);
                fs->actNumFile--;
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

    if(file != NULL){ // il file esiste

        if (flags == 1 || flags == 3){    // O_CREATE oppure O_CREATE | O_LOCK
            EXIT_ON(pthread_mutex_unlock(&fs->fs_lock), != 0);
            return E_ALR_EX;   
        }

        if(flags == 0){ // nessun flag
            f_startWrite(file);
            list_insert( &file->openedBy, clientFd);
            f_doneWrite(file);
            EXIT_ON(pthread_mutex_unlock(&fs->fs_lock), != 0);   
            return 0;
        }

        if (flags == 2){    // O_LOCK
            f_startWrite(file);
           
            if(file->lockedBy == 0){
                file->lockedBy = clientFd;
                f_doneWrite(file);
                EXIT_ON(pthread_mutex_unlock(&fs->fs_lock), != 0);   
                return 0;
            }
            else{
                f_doneWrite(file);
                EXIT_ON(pthread_mutex_unlock(&fs->fs_lock), != 0);   
                return E_ALR_LK;
            }

        }


    }

    return 0;
}

File_t * cacheEvict(FileSystem_t * fs){ // assume la lock sul fs, non ci sarà nessuno in attesa sul file ritornato
    File_t * tmp = fs->firstFile;
    File_t * prev;

    while(tmp != NULL){
        f_startRead(tmp);
        
        if(tmp->lockedBy == 0){ // se è bloccato vado avanti

            f_doneRead(tmp);
            f_startWrite(tmp);   
            fs->actSize -= tmp->size;
            fs->actNumFile--;
            fs->firstFile = tmp->next;
            f_doneWrite(tmp); // quando esco da un file in scrittura, non c'è più nessuno nemmeno in attesa
            if(fs->firstFile != NULL){
                f_startWrite(fs->firstFile);
                (fs->firstFile)->prev = NULL;
                f_doneWrite(fs->firstFile);
            } 

            return tmp;
            
        }
        prev = tmp;
        tmp = tmp->next; 
        f_doneRead(prev);
    }

    return NULL;
}

void deleteFile(File_t * tmp){
    free(tmp->path);
    if(tmp->cont != NULL) free(tmp->cont);
    list_deinit(&tmp->openedBy);
    EXIT_ON(pthread_mutex_destroy(&tmp->f_mutex), != 0);
    EXIT_ON(pthread_mutex_destroy(&tmp->f_order), != 0);
    EXIT_ON(pthread_cond_destroy(&tmp->f_go), != 0);
}

File_t * searchFile(FileSystem_t * fs, char * path){ // da cambiare con la tabella hash

    File_t * tmp = fs->firstFile;
    File_t * prev;

    while(tmp != NULL){
        f_startRead(tmp);
        if(strncmp(tmp->path, path, MAX_PATH) == 0){
            f_doneRead(tmp);
            return tmp;
        }
        prev = tmp;
        tmp = tmp->next;
        f_doneRead(prev);
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