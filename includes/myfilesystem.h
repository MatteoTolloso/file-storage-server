#ifndef _MYFILESYSTEM_H
#define _MYFILESYSTEM_H
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <myutil.h>
#include <mylinkedlist.h>
#include <unistd.h>

#define OPEN_F 1
#define READ_F 2
#define READ_N_F 3
#define WRITE_F 4
#define APPEND_T_F 5
#define LOCK_F 6
#define UNLOCK_F 7
#define CLOSE_F 8
#define REMOVE_F 9

//messages server-client
#define E_LOCK 4    // from server
#define E_NOT_EX 5  // from server
#define E_EX 6      // from server

#define MAX_PATH 1024

typedef struct File_t{
    char* path;
    int size;
    void * cont; //contenuto del file 

    struct File_t * prev;
    struct File_t * next;

    List_t* openedBy;   // lista di client che hanno il file aperto 

    int lockedBy;   // client che ha fatto la lock 

    pthread_mutex_t f_mutex; //accesso in mutua esclusione alle variabili condivise
    pthread_mutex_t f_order; //utilizzata per regolare l'accesso fair
    pthread_cond_t f_go;    //sospensione sia dei lettori che degli scrittori
    int f_activeReaders;
    int f_activeWriters;
}File_t;

typedef struct FileSystem_t{
    int actSize;    //dimensione attuale
    int actNumFile; //numero di file attuali
    int maxSize;    //massima dimensione possibile
    int maxNumFile; //massimo numero file possibile
    int maxRSize;   //massima dimensione raggiunta
    int maxRNumFile;//massimo numero di file raggiunto
    int cacheAlgoCount; // numero di esecuzioni algoritmo cache

    pthread_mutex_t fs_lock;

    File_t * firstFile;
    File_t * lastFile;

}Filesystem_t;


int fs_request_manager(Filesystem_t * fs, int clientFd, int requestType);
Filesystem_t * init_FileSystem(int maxNumFile, int maxSize);
int openFile_handler(Filesystem_t * fs, int clientFd, char * path, int flags);
File_t * searchFile(Filesystem_t * fs, char * path);




#endif /* _MYFILESYSTEM_H */