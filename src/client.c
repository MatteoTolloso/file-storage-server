#define _POSIX_C_SOURCE 200809L 
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <myutil.h>
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

#define E_INV_SCK 1 //internal
#define E_INV_FLG 2 //internal
#define E_INV_PTH 3 //internal
#define E_LOCK 4    // from server
#define E_NOT_EX 5  // from server
#define E_ALR_EX 6      // from server
#define E_BAD_RQ 7  // from server
#define E_ALR_LK 8
#define E_NO_SPACE 9


#define O_CREATE 1
#define O_LOCK 2

#define MAX_PATH 1024
#define UNIX_PATH_MAX 108

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

/* API */


int main(int argc, char ** argv){

    printf("apro la connessione : %d\n", openConnection(SOCKNAME));

   
    printf("provo ad aprire un file : %d\n", openFile("pluto_pippo ciaio", O_CREATE | O_LOCK));

    printf("provo ad aprire un file : %d\n", openFile("filepaperino", O_CREATE));
    
    printf("provo ad aprire un file : %d\n", openFile("filedue", O_CREATE));

    printf("provo ad aprire un file : %d\n", openFile("pluto_pippo ciaio", O_LOCK));


    errno = 0;
    printf("error %d, %d: \n", myerrno, errno);

    

    printf("chiudo la connesione : %d\n", closeConnection(SOCKNAME));

    return 0;

}

int openFile(const char* pth, int flags){
    
    if(flags < 0 || flags > 2) {
        myerrno = E_INV_FLG;
        return -1;
    }

    char pathname[MAX_PATH+1];
    strncpy(pathname, pth, MAX_PATH);
    pathname[MAX_PATH] = '\0';

    int len = strlen(pathname) + 1; // == sizeof(pathname)
    pathname[len] = '\0';
    int reqType = OPEN_F;
    int resp = 0;

    if(pathname == NULL || len == MAX_PATH){
        myerrno = E_INV_PTH;
        return -1;
    }

    if(write(socketfd, &reqType, sizeof(int)) != sizeof(int)){  // scrivo il tipo di richesta
        myerrno = errno;
        return -1;
    }

    if(write(socketfd, &len, sizeof(int)) != sizeof(int)){  // scrivo la lunghezza del path
        myerrno = errno;
        return -1;
    }

    if(write(socketfd, &pathname, len) != len){  // scrivo il path
        myerrno = errno;
        return -1;
    }

    if(write(socketfd, &flags, sizeof(int)) != sizeof(int)){  // scrivo i flag
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
