
#include <myfilesystem.h>

int fs_request_manager(FileSystem_t * fs, int clientFd, int requestType){

    switch (requestType){
        
        case OPEN_F:;{
            
            int flags;  
            char path[MAX_PATH]; 
            
            if (readn(clientFd, path, MAX_PATH) != MAX_PATH) return -1;    // path
            if (readn(clientFd, &flags, sizeof(int)) != sizeof(int)) return -1; // flags

            return openFile_handler(fs, clientFd, path, flags);
            
        }
        break;

        case READ_F:;{
            char path[MAX_PATH];
            if (readn(clientFd, path, MAX_PATH) != MAX_PATH) return -1;  // path

            return readFile_handler(fs, clientFd, path);
        }

        break;

        case READ_N_F:;{
            int numFile;
            if (readn(clientFd, &numFile, sizeof(int)) != sizeof(int)) return -1; // lunghezza file

            return readNFiles_handler(fs, clientFd, numFile);
        }

        break;

        case WRITE_F:;{
            int size;
            char path[MAX_PATH], * buf;
            
            if (readn(clientFd, &size, sizeof(int)) != sizeof(int)) return -1; // lunghezza file
            if (readn(clientFd, path, MAX_PATH) != MAX_PATH) return -1;  // path
            EXIT_ON(buf = malloc(size), == NULL);
            if (readn(clientFd, buf, size) != size){free(buf); return -1;} // contenuto

            //scrive nel fs il file con path e contenuto buf, rimanda i file espulsi al client
            return writeFile_handler(fs, clientFd, path, size, buf);

        }
        break;

        case APPEND_T_F:;{
            int size;
            char path[MAX_PATH], * buf;
            if (readn(clientFd, &size, sizeof(int)) != sizeof(int)) return -1; // lunghezza buffer
            if (readn(clientFd, path, MAX_PATH) != MAX_PATH) return -1;  // path
            EXIT_ON(buf = malloc(size), == NULL);
            if (readn(clientFd, buf, size) != size){free(buf); return -1;} // buffer
            
            return appendToFile_handler(fs,clientFd, path, buf, size);
        }

        break;

        case LOCK_F:;{
            char path[MAX_PATH];
            if (readn(clientFd, path, MAX_PATH) != MAX_PATH) return -1;  // path
            fprintf(stderr, "path %s\n", path);

            return lockFile_handler(fs, clientFd, path);
        }

        break;

        case UNLOCK_F:;{
            char path[MAX_PATH];
            if (readn(clientFd, path, MAX_PATH) != MAX_PATH) return -1;  // path

            return unlockFile_handler(fs, clientFd, path);
        }

        break;

        case CLOSE_F:;{
            char path[MAX_PATH];
            if (readn(clientFd, path, MAX_PATH) != MAX_PATH) return -1;  // path

            return closeFile_handler(fs, clientFd, path);
        }

        break;

        case REMOVE_F:;{
            char path[MAX_PATH];
            if (readn(clientFd, path, MAX_PATH) != MAX_PATH) return -1;  // path

            return removeFile_handler(fs, clientFd, path);
        }

        break;

        case CLOSE_ALL:;{
            return closeAll_handler(fs, clientFd);
        }

        default:
            fprintf(stderr, "Richesta non disponibile\n");
            int retVal = E_BAD_RQ;
            if (writen(clientFd, &retVal, sizeof(int)) != sizeof(int)){
                return -1;
            }

        break;

    }
    return -1;

}

int readNFiles_handler(FileSystem_t * fs, int clientFd, int numFile){

    EXIT_ON(pthread_mutex_lock(&fs->fs_lock), != 0);    // prendo lock fs

    server_log("il client %d ha richiesto la lettura di %d files", clientFd, numFile);

    // size, path, cont

    if(numFile < 0) numFile = fs->actNumFile;
    if(numFile > fs->actNumFile) numFile = fs->actNumFile;

    int retVal = 0, cont = 0;

    File_t * tmp = fs->firstFile, *next;

    retVal = 1; // seganala che stanno per arrivare file
    if (writen(clientFd, &retVal, sizeof(int)) != sizeof(int)) {retVal = E_BAD_RQ; goto readNFiles_handler_END;} 

    while(cont < numFile ){
        f_startRead(tmp);
        server_log("invio al client %d il file %s di %d byte", clientFd, tmp->path, tmp->size); 
        if (writen(clientFd, &tmp->size, sizeof(int)) != sizeof(int)) {f_doneRead(tmp), retVal = E_BAD_RQ; goto readNFiles_handler_END;}
        if (writen(clientFd, tmp->cont, tmp->size) != tmp->size) {f_doneRead(tmp), retVal = E_BAD_RQ; goto readNFiles_handler_END;}
        next = tmp->next;
        f_doneRead(tmp);
        tmp = next;
        cont++;
    }

    retVal = -1; // seganala che sono finiti i file
    if (writen(clientFd, &retVal, sizeof(int)) != sizeof(int)) {retVal = E_BAD_RQ; goto readNFiles_handler_END;} 

    retVal = 0;
    readNFiles_handler_END:
    server_log("il client %d ha completato la richiesta. Valore ritornato: %d", clientFd, retVal);
    EXIT_ON(pthread_mutex_unlock(&fs->fs_lock), != 0); 
    if (writen(clientFd, &retVal, sizeof(int)) != sizeof(int)) return -1;
    else return 0;

}

int readFile_handler(FileSystem_t * fs, int clientFd, char * path){

    EXIT_ON(pthread_mutex_lock(&fs->fs_lock), != 0);    // prendo lock fs

    server_log("il client %d ha richiesto la lettura di %s", clientFd, path);

    int retVal = 0;
    
    File_t * file = searchFile(fs, path);

    if(file == NULL){ retVal = E_NOT_EX; goto readFile_handler_END;}

    f_startRead(file);

    EXIT_ON(pthread_mutex_unlock(&fs->fs_lock), != 0); // posso rilaciare la mutex del fs

    if(!list_mem(&file->openedBy, clientFd)){ retVal = E_NOT_OPN; f_doneRead(file); goto readFile_handler_END;}

    if(file->lockedBy != 0 && file->lockedBy != clientFd){retVal = E_LOCK; f_doneRead(file); goto readFile_handler_END;}

    retVal = 1; // seganala che stanno per arrivare file
    if (writen(clientFd, &retVal, sizeof(int)) != sizeof(int)) {f_doneRead(file), retVal = E_BAD_RQ; goto readFile_handler_END;} 

    server_log("invio al client %d il file %s di %d byte", clientFd, file->path, file->size);
        
    if (writen(clientFd, &file->size, sizeof(int)) != sizeof(int)) {f_doneRead(file), retVal = E_BAD_RQ; goto readFile_handler_END;}
    if (writen(clientFd, file->cont, file->size) != file->size) {f_doneRead(file), retVal = E_BAD_RQ; goto readFile_handler_END;}

    f_doneRead(file);

    retVal = 0;
    readFile_handler_END:
    server_log("il client %d ha completato la richiesta. Valore ritornato: %d", clientFd, retVal);
    if (writen(clientFd, &retVal, sizeof(int)) != sizeof(int)) return -1;
    else return 0;

}

int removeFile_handler(FileSystem_t * fs, int clientFd, char * path){   // da testare bene ( e scrivere il lato client)
    
    EXIT_ON(pthread_mutex_lock(&fs->fs_lock), != 0);    // prendo lock fs

    server_log("il client %d ha richiesto l'eliminazione di %s", clientFd, path);

    int retVal = 0;
    
    File_t * file = searchFile(fs, path);

    if(file == NULL){ retVal = E_NOT_EX; goto removeFile_handler_END;}

    f_startWrite(file);

    if(file->lockedBy != clientFd){f_doneWrite(file); retVal = E_LOCK; goto removeFile_handler_END;}

    fs->actSize -= file->size;
    fs->actNumFile--;

    if(file->next != NULL) {
        f_startWrite(file->next);
        (file->next)->prev = file->prev;
        f_doneWrite(file->next);
    }
    if(file->prev != NULL) {
        f_startWrite(file->prev);
        (file->prev)->next = file->next;
        f_doneWrite(file->prev);
    }

    if (fs->firstFile == file) fs->firstFile = file->next;
    if (fs->lastFile == file) fs->lastFile = file->prev;

    
    f_doneWrite(file);
    deleteFile(file);

    removeFile_handler_END:
    server_log("il client %d ha completato la richiesta. Valore ritornato: %d", clientFd, retVal);
    EXIT_ON(pthread_mutex_unlock(&fs->fs_lock), != 0); 
    if (writen(clientFd, &retVal, sizeof(int)) != sizeof(int)) return -1;
    else return 0;
}

int closeFile_handler(FileSystem_t * fs, int clientFd, char * path){

    EXIT_ON(pthread_mutex_lock(&fs->fs_lock), != 0);    // prendo lock fs

    server_log("il client %d ha richiesto la chiusura di %s", clientFd, path);

    int retVal = 0;
    
    File_t * file = searchFile(fs, path);

    if(file == NULL){ retVal = E_NOT_EX; goto closeFile_handler_END;}

    f_startWrite(file);

    list_remove(&file->openedBy, clientFd);

    f_doneWrite(file);

    retVal = 0;
    closeFile_handler_END:
    server_log("il client %d ha completato la richiesta. Valore ritornato: %d", clientFd, retVal);
    EXIT_ON(pthread_mutex_unlock(&fs->fs_lock), != 0);
    if (writen(clientFd, &retVal, sizeof(int)) != sizeof(int)) return -1;
    else return 0;

}

int unlockFile_handler(FileSystem_t * fs, int clientFd, char * path){

    EXIT_ON(pthread_mutex_lock(&fs->fs_lock), != 0);    // prendo lock fs

    server_log("il client %d ha richiesto una unlock su %s", clientFd, path);

    int retVal = 0;
    
    File_t * file = searchFile(fs, path);

    if(file == NULL){ retVal = E_NOT_EX; goto unlockFile_handler_END;}

    f_startWrite(file);

    if(!list_mem(&file->openedBy, clientFd)){ f_doneWrite(file), retVal = E_NOT_OPN; goto unlockFile_handler_END;}

    if(file->lockedBy != clientFd){ f_doneWrite(file); retVal = E_LOCK ; goto unlockFile_handler_END;}

    if(file->lockedBy == 0){ f_doneWrite(file), retVal = 0; goto unlockFile_handler_END;}

    file->lockedBy = 0;


    retVal = 0;
    unlockFile_handler_END:
    server_log("il client %d ha completato la richiesta. Valore ritornato: %d", clientFd, retVal);
    EXIT_ON(pthread_mutex_unlock(&fs->fs_lock), != 0);
    if (writen(clientFd, &retVal, sizeof(int)) != sizeof(int)) return -1;
    else return 0;

}

int closeAll_handler(FileSystem_t * fs, int clientFd){

    EXIT_ON(pthread_mutex_lock(&fs->fs_lock), != 0);

    server_log("chiudo il client %d e tutti i file aperti", clientFd);

    File_t * next,* tmp = fs->firstFile;

    while(tmp){
        f_startWrite(tmp);
        list_remove(&tmp->openedBy, clientFd);
        if(tmp->lockedBy == clientFd) tmp->lockedBy = 0;
        next = tmp->next;
        f_doneWrite(tmp);
        tmp = next;
    }
    
    EXIT_ON(pthread_mutex_unlock(&fs->fs_lock), != 0);

    return 0;

}

int lockFile_handler(FileSystem_t * fs, int clientFd, char * path){

    EXIT_ON(pthread_mutex_lock(&fs->fs_lock), != 0);    // prendo lock fs

    server_log("il client %d ha richiesto una lock su %s", clientFd, path);

    int retVal = 0;
    
    File_t * file = searchFile(fs, path);

    if(file == NULL){ retVal = E_NOT_EX; goto lockFile_handler_END;}

    f_startWrite(file);

    if(!list_mem(&file->openedBy, clientFd)){ f_doneWrite(file), retVal = E_NOT_OPN; goto lockFile_handler_END;}

    if(file->lockedBy == clientFd){ f_doneWrite(file); retVal = 0 ; goto lockFile_handler_END;}

    if(file->lockedBy == 0){ 
        file->lockedBy = clientFd;
        f_doneWrite(file);
        retVal = 0;
        goto lockFile_handler_END;
    }

    if(file->lockedBy != 0 && file->lockedBy != clientFd){ 
        f_doneWrite(file); 
        retVal = E_ALR_LK; 
        goto lockFile_handler_END;
    }

    retVal = 0;
    lockFile_handler_END:
    server_log("il client %d ha completato la richiesta. Valore ritornato: %d", clientFd, retVal);
    EXIT_ON(pthread_mutex_unlock(&fs->fs_lock), != 0);
    if (writen(clientFd, &retVal, sizeof(int)) != sizeof(int)) return -1;
    else return 0;

}

int appendToFile_handler(FileSystem_t * fs,int clientFd, char * path, char * buf, int size){
    
    EXIT_ON(pthread_mutex_lock(&fs->fs_lock), != 0);    // prendo lock fs

    server_log("il client %d ha richiesto una append sul file %s di %d byte", clientFd, path, size);

    int retVal = 0;
    
    File_t * file = searchFile(fs, path);

    if(file == NULL){ retVal = E_NOT_EX; goto appendToFile_handler_END;}

    f_startRead(file);

    if(!list_mem(&file->openedBy, clientFd)){f_doneRead(file); retVal = E_NOT_OPN; goto appendToFile_handler_END;}

    if(file->lockedBy != 0 && file->lockedBy != clientFd){f_doneRead(file); retVal = E_LOCK; goto appendToFile_handler_END;}

    if(size + file->size > fs->maxSize){f_doneRead(file); retVal = E_NO_SPACE; goto appendToFile_handler_END;}
    
    f_doneRead(file);

    retVal = 0;
    while((fs->maxSize - fs->actSize < size)){
        
        if (retVal == 0){
            retVal = 1; // seganala che stanno per arrivare file
            if (writen(clientFd, &retVal, sizeof(int)) != sizeof(int)) perror("i file verranno persi"); 
        }
        File_t * tmp = cacheEvict(fs, file, F_SIZE);  // scollega il file dal fs e non ci sono scrittori o lettori attivi o in attesa
        
        server_log("invio al client %d il file %s di %d byte", clientFd, tmp->path, tmp->size);
        
        if (writen(clientFd, &tmp->size, sizeof(int)) != sizeof(int)) perror("i file verranno persi"); 
        if (writen(clientFd, tmp->path, MAX_PATH) != MAX_PATH) perror("i file verranno persi"); 
        if (writen(clientFd, tmp->cont, tmp->size) != tmp->size) perror("i file verranno persi"); 

        server_log("file %s eliminato dal server", tmp->path);
        deleteFile(tmp);

    }
    if (retVal == 1){   // se ho inviato i file devo segnalare che sono finiti
        retVal = -1; // seganala che i file sono finiti
        if (writen(clientFd, &retVal, sizeof(int)) != sizeof(int)) perror("errore client"); 
    }

    f_startWrite(file);

    EXIT_ON(file->cont = realloc(file->cont, file->size + size), == NULL);
    memcpy((char*)file->cont + file->size, buf, size);
    file->size += size;
    f_doneWrite(file);
    fs->actSize+=size;
    if(fs->actSize > fs->maxRSize) fs->maxRSize = fs->actSize;


    retVal = 0;
    appendToFile_handler_END:
    server_log("il client %d ha completato la richiesta. Valore ritornato: %d", clientFd, retVal);
    free(buf);
    EXIT_ON(pthread_mutex_unlock(&fs->fs_lock), != 0);
    if (writen(clientFd, &retVal, sizeof(int)) != sizeof(int)) return -1;
    else return 0;

}

int writeFile_handler(FileSystem_t * fs, int clientFd, char* path, int size, char * buf){

    EXIT_ON(pthread_mutex_lock(&fs->fs_lock), != 0);    // prendo lock fs
    
    server_log("il client %d ha richiesto una write sul file %s di %d byte", clientFd, path, size);

    int retVal = 0; // valore di ritorno che corrisponde a errno per il client
    File_t * file = searchFile(fs, path); // cerco il file con quel path
    
    if(file == NULL){
        retVal = E_NOT_EX;
        goto writeFile_handler_END; //test da fare: lanciare il client
    }

    f_startRead(file); // devo levvere il file
    
    if(!list_mem(&file->openedBy, clientFd)){    // controllo se ho aperto il file
        f_doneRead(file);
        retVal = E_NOT_OPN;
        goto writeFile_handler_END;
    }

    if(clientFd != file->lockedBy ){    // controllo se ho la lock sul file
        f_doneRead(file);
        retVal =  E_LOCK;
        goto writeFile_handler_END;
    }

    if(file->size != 0){        // controllo se il file è stato già scritto
        f_doneRead(file);
        retVal = E_BAD_RQ;
        goto writeFile_handler_END;
    }
    
    if(fs->maxSize < size){  // non riuscirò mai a trovare lo spazio necessario
        f_doneRead(file);
        retVal = E_NO_SPACE;
        goto writeFile_handler_END;
    }
    
    f_doneRead(file);
     
    // fase di evict e salvataggio dei file da mandare al client
    retVal = 0;
    while((fs->maxSize - fs->actSize < size)){  // se lo spazio libero è minore di quello che devo inserire
        
        if (retVal == 0){
            retVal = 1; // seganala che stanno per arrivare file
            if (writen(clientFd, &retVal, sizeof(int)) != sizeof(int)) perror("i file verranno persi"); 
        }
        File_t * tmp = cacheEvict(fs, file, F_SIZE);  // scollega il file dal fs e non ci sono scrittori o lettori attivi o in attesa
        
        server_log("invio al client %d il file %s di %d byte", clientFd, tmp->path, tmp->size);
        
        if (writen(clientFd, &tmp->size, sizeof(int)) != sizeof(int))perror("i file verranno persi"); 
        if (writen(clientFd, tmp->path, MAX_PATH) != MAX_PATH) perror("i file verranno persi"); 
        if (writen(clientFd, tmp->cont, tmp->size) != tmp->size) perror("i file verranno persi"); 

        server_log("file %s eliminato dal server", tmp->path);
        deleteFile(tmp);
        
    }
    if(retVal == 1){    // se ho inviato file, devo segnalare che i file sono finiti
        retVal = -1;    // i file sono finiti
        if (writen(clientFd, &retVal, sizeof(int)) != sizeof(int)) perror("errore client");
    }
    
    // scrittura nel file 

    f_startWrite(file);
    file->cont = buf;
    file->size = size;
    fs->actSize += size;
    if(fs->actSize >fs->maxRSize) fs->maxRSize = fs->actSize;
    f_doneWrite(file);
    //fprintf(stderr, "scritto il file: %s con: %s\n", file->path, (char*)file->cont);

    // messaggio finale per il client
    retVal = 0;
    writeFile_handler_END:
    if(retVal != 0) free(buf);
    server_log("il client %d ha completato la richiesta. Valore ritornato: %d", clientFd, retVal);
    EXIT_ON(pthread_mutex_unlock(&fs->fs_lock), != 0);
    if (writen(clientFd, &retVal, sizeof(int)) != sizeof(int)) return -1;
    else return 0;

}

int openFile_handler(FileSystem_t * fs, int clientFd, char * path, int flags){
    
    EXIT_ON(pthread_mutex_lock(&fs->fs_lock), != 0);    // prendo lock fs

    server_log("il client %d ha richiesto una open sul file %s con flag %d", clientFd, path, flags);

    int retVal = 0;
    File_t * file = searchFile(fs, path); // cerco il file 
    
    // agisco in base ai flag

    if(file == NULL){     // file non esisteva
        
        if(flags == 0 || flags == 2){     //nessun flag o solo O_LOCK
            retVal = E_NOT_EX; 
            goto openFile_handler_END;
        }

        // devo aggiungere un file
        
        if(fs->actNumFile + 1 == fs->maxNumFile){   // rimpiazzamento, da aggiungere l'invio al client
            File_t * tmp = cacheEvict(fs, NULL, 0);
            assert(tmp != NULL);
            fprintf(stderr, "evict del file: %s per massimo numero di file\n", tmp->path);
            deleteFile(tmp);
        }

        File_t * newfile  = init_File(path, clientFd); 
        
        f_startWrite(newfile);
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
        list_insert(&newfile->openedBy, clientFd);
        f_doneWrite(newfile);
        fs->actNumFile++;
        if(fs->actNumFile > fs->maxRNumFile) fs->maxRNumFile = fs->actNumFile;        
        retVal = 0;   
        goto openFile_handler_END;   
    }

    if(file != NULL){ // il file esiste

        if (flags == 1 || flags == 3){    // O_CREATE oppure O_CREATE | O_LOCK
            retVal = E_ALR_EX;   
            goto openFile_handler_END;   

        }

        if(flags == 0){ // nessun flag
            f_startWrite(file);
            list_insert( &file->openedBy, clientFd);
            f_doneWrite(file);  
            retVal = 0;   
            goto openFile_handler_END;   

        }

        if (flags == 2){    // O_LOCK
            f_startWrite(file);
           
            if(file->lockedBy == 0){
                file->lockedBy = clientFd;
                list_insert(&file->openedBy, clientFd);
                f_doneWrite(file);
                retVal = 0;   
                goto openFile_handler_END;   
            }
            else{
                f_doneWrite(file);
                retVal = E_ALR_LK;
                goto openFile_handler_END;   
            }
        }
    }
    
    openFile_handler_END:
    server_log("il client %d ha completato la richiesta. Valore ritornato: %d", clientFd, retVal);
    EXIT_ON(pthread_mutex_unlock(&fs->fs_lock), != 0);   
    if (writen(clientFd, &retVal, sizeof(int)) != sizeof(int)) return -1;
    else return 0;
}

File_t * cacheEvict(FileSystem_t * fs, File_t * f, int flag){ 
    // assume la lock sul fs, non ci sarà nessuno in attesa sul file ritornato
    // nessun altro file deve essere in scrittura o lettura
    // f indica un file escluso dall'evict
    // flag indica se devo recuperare spazio o se mi basta eliminare un file qualsiasi
    File_t * tmp = fs->firstFile;
    File_t * prev;

    while(tmp != NULL){
        f_startRead(tmp);

        
        if(tmp != f && ((tmp->size != 0) || !flag)){ // deve essere diverso dal file che sto scrivendo

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
    EXIT_ON("evict fail", );
    return NULL;
}

void deleteFile(File_t * tmp){
    if(tmp->cont != NULL) free(tmp->cont);
    list_deinit(&tmp->openedBy);
    EXIT_ON(pthread_mutex_destroy(&tmp->f_mutex), != 0);
    EXIT_ON(pthread_mutex_destroy(&tmp->f_order), != 0);
    EXIT_ON(pthread_cond_destroy(&tmp->f_go), != 0);
    free(tmp);
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

void deinit_FileSystem(FileSystem_t * fs){
    // assume che non ci sia più nessun thread worker, quindi ignora tutte le lock

    File_t *prev, * tmp = fs->firstFile;

    while(tmp != NULL){
        prev = tmp;
        tmp = tmp->next;
        deleteFile(prev);
    }

    EXIT_ON(pthread_mutex_destroy(&fs->fs_lock), != 0);

    free(fs);
}

File_t * init_File(char* path, int clientFd){
    
    File_t * file;
    EXIT_ON(file = malloc(sizeof(File_t)), == NULL);

    
    EXIT_ON( strncpy(file->path, path, MAX_PATH), == NULL);
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