#ifndef _MYFILESYSTEM_H
#define _MYFILESYSTEM_H
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <myutil.h>
#include <mylinkedlist.h>
#include <unistd.h>
#include <assert.h>

#define OPEN_F 1
#define READ_F 2
#define READ_N_F 3
#define WRITE_F 4
#define APPEND_T_F 5
#define LOCK_F 6
#define UNLOCK_F 7
#define CLOSE_F 8
#define REMOVE_F 9
#define CLOSE_ALL 10

//messages server-client
#define RET_FILE 1  
#define E_INV_FLG 2 //internal
#define E_INV_PTH 3 //internal
#define E_LOCK 4    // from server
#define E_NOT_EX 5  // from server
#define E_ALR_EX 6      // from server
#define E_BAD_RQ 7  // from server
#define E_ALR_LK 8
#define E_NO_SPACE 9
#define E_NOT_OPN 10 
#define E_INV_SCK 11 //internal


#define MAX_PATH 1024
#define ENDVAL "_end"

#define F_SIZE 1
typedef struct File_t{
    char path[MAX_PATH+1];
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

}FileSystem_t;


int fs_request_manager(FileSystem_t * fs, int clientFd, int requestType);
FileSystem_t * init_FileSystem(int maxNumFile, int maxSize);
int openFile_handler(FileSystem_t * fs, int clientFd, char * path, int flags);
File_t * searchFile(FileSystem_t * fs, char * path);
int writen(int fd, void *ptr, size_t n);
int readn(int fd, void *ptr, size_t n);
File_t * init_File(char* path, int clientFd);
void printFs(FileSystem_t * fs);
int writeFile_handler(FileSystem_t * fs, int clientFd, char* path, int size, char * buf);
void f_startWrite(File_t * file);
void f_doneWrite(File_t * file);
void deleteFile(File_t * tmp);
File_t * cacheEvict(FileSystem_t * fs, File_t * f, int flag);
void f_startRead(File_t * file);
void f_doneRead(File_t * file);
void deinit_FileSystem(FileSystem_t * fs);
int appendToFile_handler(FileSystem_t * fs,int clientFd, char * path, char * buf, int size);
int lockFile_handler(FileSystem_t * fs, int clientFd, char * path);
int closeAll_handler(FileSystem_t * fs, int clientFd);
int unlockFile_handler(FileSystem_t * fs, int clientFd, char * path);
int closeFile_handler(FileSystem_t * fs, int clientFd, char * path);
int removeFile_handler(FileSystem_t * fs, int clientFd, char * path);
int readFile_handler(FileSystem_t * fs, int clientFd, char * path);
int readNFiles_handler(FileSystem_t * fs, int clientFd, int numFile);


#endif /* _MYFILESYSTEM_H */