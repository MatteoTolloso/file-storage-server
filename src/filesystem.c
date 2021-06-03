// todo filesistem.h

#include <stdio.h>
#include <pthread.h>
#include <string.h>

typedef struct _file{
    char* path;
    int size;
    void * cont; //contenuto del file 

    File_t * prev;
    File_t * next;

    pthread_mutex_t f_mutex; // protocollo lettore scrittore
    pthread_mutex_t f_order;
    pthread_cond_t f_go;
    int active_readers;
    int active_writers;

}File_t;

typedef struct _fs{
    int numberOfFile;
    int maxSize;
    int maxNumFile;
    pthread_mutex_t fs_lock;
    pthread_cond_t fs_cond; // qualsiasi operazione che modifica 

    File_t * firstFile;
    File_t * lastFile;

}Filesystem_t;