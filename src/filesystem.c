// todo filesistem.h

#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <myutil.h>

typedef struct _file{
    char* path;
    int size;
    void * cont; //contenuto del file 

    File_t * prev;
    File_t * next;

    pthread_mutex_t f_mutex; //accesso in mutua esclusione alle variabili condivise
    pthread_mutex_t f_order; //utilizzata per regolare l'accesso fair
    pthread_cond_t f_go;    //sospensione sia dei lettori che degli scrittori
    int f_activeReaders;
    int f_activeWriters;

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

void file_system_init(){


}

void f_startRead(File_t * file){
    EXIT_ON(pthread_mutex_lock(&file->f_order), != 0); // sono in fila
    EXIT_ON(pthread_mutex_lock(&file->f_mutex), != 0 ); // sono in sezione critica
    while(f_activeWriters > 0){     // se ci sono scrittori attivi mi sospendo (ma mantenfo il posto in fila)
        EXIT_ON(pthread_cond_wait(&file->f_go, &file->f_mutex), != 0 );
    }
    f_activeReaders++;
    EXIT_ON(ptherad_mutex_unlock(&file->f_order), != 0); // rilascio il posto in fila
    EXIT_ON(pthread_mutex_unlock(&file->f_mutex), != 0); // rilascio la mutex
}

void f_doneRead(File_t * file){
    EXIT_ON(pthread_mutex_lock(&file->f_mutex), != 0);
    f_activeReaders--;
    if(f_activeReaders == 0){
        EXIT_ON( pthread_cond_signal(&file->f_go), != 0);   // faccio entrare il prossimo in fila (se non ci sono lettori attivi è inutile)
    }
    EXIT_ON( pthread_muttex_unlock(&file->f_mutex), != 0);
}

void f_startWrite(File_t * file){
    EXIT_ON(pthread_mutex_lock(&file->f_order), != 0); // sono in fila
    EXIT_ON(pthread_mutex_lock(&file->f_mutex), != 0 ); // sono in sezione critica
    while(f_activeReaders > 0 || f_activeWriters > 0){     // se ci sono scrittori o lettori attivi mi sospendo (ma mantenfo il posto in fila)
        EXIT_ON(pthread_cond_wait(&file->f_go, &file->f_mutex), != 0 );
    }
    f_activeWritesr++;
    EXIT_ON(ptherad_mutex_unlock(&file->f_order), != 0); // rilascio il posto in fila
    EXIT_ON(pthread_mutex_unlock(&file->f_mutex), != 0); // rilascio la mutex
}

void f_doneWrite(File_t * file){
    EXIT_ON(pthread_mutex_lock(&file->f_mutex), != 0);
    f_activeWriters--;
    EXIT_ON( pthread_cond_signal(&file->f_go), != 0);
    EXIT_ON( pthread_muttex_unlock(&file->f_mutex), != 0);
}
