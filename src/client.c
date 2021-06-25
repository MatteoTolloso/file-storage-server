#define _POSIX_C_SOURCE 200809L 
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <myutil.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <sys/uio.h>
#include <sys/un.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//lo deve prendere da riga di comando 
#define SOCKNAME "./mysock"

/* API */

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


#define O_CREATE 1
#define O_LOCK 2

#define MAX_PATH 1024
#define UNIX_PATH_MAX 108
#define BUF_SIZE 2048


#define OPEN_F 1
#define READ_F 2
#define READ_N_F 3
#define WRITE_F 4
#define APPEND_T_F 5
#define LOCK_F 6
#define UNLOCK_F 7
#define CLOSE_F 8
#define REMOVE_F 9

int socketfd, myerrno;
char __sockname[108];

int openConnection(const char* sockname/*, int msec, const struct timespec abstime */);
int closeConnection(const char* sockname);
int openFile(const char* pathname, int flags);
int writeFile(char* pathname, char * dirname);
int appendToFile(char * pathname, void * buf, size_t size, char * dirname);
int lockFile(char * pathname);
int unlockFile(char * pathname);
int closeFile(char * pathname);

/* API */


int main(int argc, char ** argv){

    myerrno = 0;
    errno = 0;

    
    openConnection(SOCKNAME);

    
    openFile("./test2.txt", O_CREATE | O_LOCK);
    writeFile("./test2.txt", ".");
    fprintf(stderr,"myerrno: %d \n", myerrno);

    closeFile("./test2.txt");


    myerrno = 0;
    lockFile("./test2.txt");
    fprintf(stderr,"myerrno: %d \n", myerrno);

    unlockFile("test2.txt");
   
   /*
    openFile("./test1.txt", O_CREATE | O_LOCK);
    fprintf(stderr,"myerrno: %d \n", myerrno);

    writeFile("./test1.txt", ".");
    fprintf(stderr,"myerrno: %d \n", myerrno);

    
    appendToFile("./test1.txt", buf, strlen(buf), ".");
    fprintf(stderr,"myerrno: %d \n", myerrno);
   

    closeConnection(SOCKNAME);
    */
    return 0;

}

int closeFile(char * pathname){
    if(pathname == NULL){ myerrno = E_INV_PTH; return -1;}

    char pth[MAX_PATH];
    strncpy(pth, pathname, MAX_PATH);
    pth[MAX_PATH-1] = '\0';
    int reqType = CLOSE_F, resp = 0;

    if(write(socketfd, &reqType, sizeof(int)) != sizeof(int)){  // scrivo il tipo di richesta
        myerrno = errno;
        return -1;
    }

    if(write(socketfd, pth, MAX_PATH) != MAX_PATH){  // scrivo il path
        myerrno = errno;
        return -1;
    }

    if(read(socketfd, &resp, sizeof(int)) != sizeof(int)){  // leggo la risposta
        myerrno = errno;
        return -1;
    }

    if(resp == 0){
        return 0;
    }
    else{
        myerrno = resp;
        return -1;
    }

}
int unlockFile(char * pathname){
    if(pathname == NULL){ myerrno = E_INV_PTH; return -1;}

    char pth[MAX_PATH];
    strncpy(pth, pathname, MAX_PATH);
    pth[MAX_PATH-1] = '\0';
    int reqType = UNLOCK_F, resp = 0;

    if(write(socketfd, &reqType, sizeof(int)) != sizeof(int)){  // scrivo il tipo di richesta
        myerrno = errno;
        return -1;
    }

    if(write(socketfd, pth, MAX_PATH) != MAX_PATH){  // scrivo il path
        myerrno = errno;
        return -1;
    }

    if(read(socketfd, &resp, sizeof(int)) != sizeof(int)){  // leggo la risposta
        myerrno = errno;
        return -1;
    }

    if(resp == 0){
        return 0;
    }
    else{
        myerrno = resp;
        return -1;
    }

}

int lockFile(char * pathname){
    if(pathname == NULL){ myerrno = E_INV_PTH; return -1;}

    char pth[MAX_PATH];
    strncpy(pth, pathname, MAX_PATH);
    pth[MAX_PATH-1] = '\0';
    int reqType = LOCK_F, resp = 0;

    if(write(socketfd, &reqType, sizeof(int)) != sizeof(int)){  // scrivo il tipo di richesta
        myerrno = errno;
        return -1;
    }

    if(write(socketfd, pth, MAX_PATH) != MAX_PATH){  // scrivo il path
        myerrno = errno;
        return -1;
    }

    if(read(socketfd, &resp, sizeof(int)) != sizeof(int)){  // leggo la risposta
        myerrno = errno;
        return -1;
    }

    if(resp == 0){
        return 0;
    }
    else{
        myerrno = resp;
        return -1;
    }

}

int appendToFile(char * pathname, void * buf, size_t size, char * dirname){
    
    if(pathname == NULL){ myerrno = E_INV_PTH; return -1;}
    if(buf == NULL || size < 1){ myerrno = E_BAD_RQ; return -1;}

    char pth[MAX_PATH];
    strncpy(pth, pathname, MAX_PATH);
    pth[MAX_PATH-1] = '\0';
    int reqType = APPEND_T_F, resp = 0;

    if(write(socketfd, &reqType, sizeof(int)) != sizeof(int)){  // scrivo il tipo di richesta
        myerrno = errno;
        return -1;
    }

    if(write(socketfd, &size, sizeof(int)) != sizeof(int)){  // scrivo la size
        myerrno = errno;
        return -1;
    }
    fprintf(stderr, "size inviata %ld\n", size);
    
    if(write(socketfd, pth, MAX_PATH) != MAX_PATH){  // scrivo il path
        myerrno = errno;
        return -1;
    }
    fprintf(stderr, "path inviato %s\n",(char*) pth);
    
    if(write(socketfd, buf, size) != size){  // scrivo il buffer
        myerrno = errno;
        return -1;
    }
    fprintf(stderr, "buffer inviato %s\n",(char*) buf);

    
    
    if(read(socketfd, &resp, sizeof(int)) != sizeof(int)){  // leggo la risposta
        myerrno = errno;
        return -1;
    }
    fprintf(stderr, "risposta letta dalla pipe %d\n", resp);

    // lettura file di risposta
    
    if(resp == 1){ // devo leggere N file: size path cont
        FILE* outFile;
        fprintf(stderr, "leggo i file di risposta\n");
        while(1){

            if(read(socketfd, &size, sizeof(int)) != sizeof(int)){ myerrno = errno; return -1;} // leggo la size, se è 0 ho finit
            fprintf(stderr, "size di ritorno letta %ld\n", size);
            if(size == 0){
                return 0;
            }
            
            if(read(socketfd, pth, MAX_PATH) != MAX_PATH){ myerrno = errno; return -1;} // leggo il path
            fprintf(stderr, "path di ritorno letto %s\n", pth);

            EXIT_ON(buf = malloc(size), == NULL); // alloco il buffer
            
            if(read(socketfd, buf, size) != size){ myerrno = errno; free(buf); return -1;} // leggo il contenuto
            fprintf(stderr, "contenuto letto %s\n", (char*)buf);


            if (( outFile = fopen(pth, "wb")) == NULL){
                myerrno = errno;
                fprintf(stderr,"impossibile aprire o creare file\n");
            }
            
            if(fwrite(buf, 1, size, outFile) != size){  // scrivo contenuto su file
                myerrno = errno;
                free(buf);
                fclose(outFile);
                return -1;
            }
            fprintf(stderr, "scritto\n");

            free(buf);
            fclose(outFile);
        }
    }
    else return 0;


}

int writeFile(char* pathname, char * dirname){

    fprintf(stderr, "writefile path %s, dir %s\n", pathname, dirname);
    if(pathname == NULL){
        myerrno = E_INV_PTH;
        return -1;
    }

    char pth[MAX_PATH], *buf;
    strncpy(pth, pathname, MAX_PATH);
    pth[MAX_PATH-1] = '\0';
    int reqType = WRITE_F, resp = 0;
    FILE * inFile;

    if((inFile = fopen(pth, "rb")) == NULL){
        myerrno = errno;
        fprintf(stderr,"impossibile aprire file\n");
        return -1;
    }
 
    // leggo il file
    size_t size = 0;

    fseek(inFile, 0, SEEK_END);
    size = ftell(inFile);
    fseek(inFile, 0, SEEK_SET);
    EXIT_ON(buf = malloc(size), == NULL);
    if( fread(buf, 1, size, inFile) != size){
        fprintf(stderr, "impossibile leggere il file\n");
        myerrno = errno;
        free(buf);
        return -1;
    }
    
    fclose(inFile);
    fprintf(stderr, "debug contenuto file: %s, size: %ld\n", buf, size);

    if(write(socketfd, &reqType, sizeof(int)) != sizeof(int)){  // scrivo il tipo di richesta
        myerrno = errno;
        return -1;
    }

    if(write(socketfd, &size, sizeof(int)) != sizeof(int)){  // scrivo la size
        myerrno = errno;
        return -1;
    }
    if(write(socketfd, pth, MAX_PATH) != MAX_PATH){  // scrivo il path
        myerrno = errno;
        return -1;
    }
    if(write(socketfd, buf, size) != size){  // scrivo il contenuto
        myerrno = errno;
        return -1;
    }
    free(buf);

    if(read(socketfd, &resp, sizeof(int)) != sizeof(int)){  // leggo la risposta
        myerrno = errno;
        return -1;
    }

    if(resp == 0){
        return 0;
    }

    // lettura file di risposta
    
    if(resp == 1){ // devo leggere N file: size path cont
        FILE* outFile;
        fprintf(stderr, "leggo i file di risposta\n");
        while(1){

            if(read(socketfd, &size, sizeof(int)) != sizeof(int)){ myerrno = errno; return -1;} // leggo la size, se è 0 ho finit
            fprintf(stderr, "size di ritorno letta %ld\n", size);
            if(size == 0){
                return 0;
            }
            
            if(read(socketfd, pth, MAX_PATH) != MAX_PATH){ myerrno = errno; return -1;} // leggo il path
            fprintf(stderr, "path di ritorno letto %s\n", pth);

            EXIT_ON(buf = malloc(size), == NULL); // alloco il buffer
            
            if(read(socketfd, buf, size) != size){ myerrno = errno; free(buf); return -1;} // leggo il contenuto
            fprintf(stderr, "contenuto letto %s\n", buf);


            if (( outFile = fopen(pth, "wb")) == NULL){
                perror("open");
                myerrno = errno;
                fprintf(stderr,"impossibile aprire o creare file\n");
            }
            
            if(fwrite(buf, 1, size, outFile) != size){  // scrivo contenuto su file
                myerrno = errno;
                free(buf);
                fclose(outFile);
                return -1;
            }
            fprintf(stderr, "scritto\n");

            free(buf);
            fclose(outFile);

        }
    }
    else {
        myerrno = resp;
        return -1;
    }
}

int openFile(const char* pathname, int flags){

    fprintf(stderr, "openfile path %s, flag %d\n", pathname, flags);

    if(flags < 0 || flags > 3) {
        myerrno = E_INV_FLG;
        return -1;
    }

    char pth[MAX_PATH];
    strncpy(pth, pathname, MAX_PATH);
    pth[MAX_PATH-1] = '\0';
    int reqType = OPEN_F;
    int resp = 0;

    if(pth == NULL){
        myerrno = E_INV_PTH;
        return -1;
    }

    if(write(socketfd, &reqType, sizeof(int)) != sizeof(int)){  // scrivo il tipo di richesta
        myerrno = errno;
        return -1;
    }
    fprintf(stderr, "c1\n");

    if(write(socketfd, &pth, MAX_PATH) != MAX_PATH){  // scrivo il path
        myerrno = errno;
        return -1;
    }
    fprintf(stderr, "c2\n");


    if(write(socketfd, &flags, sizeof(int)) != sizeof(int)){  // scrivo i flag
        myerrno = errno;
        return -1;
    }
    fprintf(stderr, "c3\n");


    if(read(socketfd, &resp, sizeof(int)) != sizeof(int)){  // leggo la risposta
        myerrno = errno;
        return -1;
    }
    fprintf(stderr, "c4\n");


    if(resp == 0){
        return 0;
    }
    else{
        myerrno = resp;
        return -1;
    }

}


int openConnection(const char* sockname /*, int msec, const struct timespec abstime*/){

    // todo 
        //msec
        //abstime
    
    if (sockname == NULL || strnlen(sockname, UNIX_PATH_MAX) >= UNIX_PATH_MAX) {
        myerrno = E_INV_SCK;
        return -1;
    }
    strcpy(__sockname, sockname);
    struct sockaddr_un server_addr;
    socketfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if(socketfd == -1) {
        myerrno = errno;
        return -1;
    }
    if (memset(&server_addr, 0, sizeof(server_addr)) == NULL) return -1;

    server_addr.sun_family = AF_UNIX;
    strcpy(server_addr.sun_path, sockname);

    if (connect(socketfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) != 0) {
        myerrno = errno;
        return -1;
    }

    return 0;

}

int closeConnection(const char* sockname){

    if (sockname == NULL || (strncmp(__sockname, sockname, UNIX_PATH_MAX) != 0)) {
        myerrno = E_INV_SCK;
        return -1;
    }
    close(socketfd);
    return 0;

}
