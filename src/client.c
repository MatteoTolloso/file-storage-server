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
#define E_EX 6      // from server

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

    printf("result : %d\n", openConnection(SOCKNAME));

    sleep(2);

    printf("result : %d\n", closeConnection(SOCKNAME));

    sleep(3);

    return 0;

}

int openFile(const char* pathname, int flags){
    
    if(flags < 0 || flags > 2) {
        myerrno = E_INV_FLG;
        return -1;
    }

    int len = strnlen(pathname, MAX_PATH); // == sizeof(pathname)
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

    if(write(socketfd, &pathname, sizeof(pathname)) != sizeof(pathname)){  // scrivo il path
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

    switch (resp){
        case 4: // file già lock
            myerrno = E_LOCK;
            return -1;
            break;
        
        case 5: //file non esiste
            myerrno = E_NOT_EX;
            return -1;
            break;
        
        case 6:
            myerrno = E_EX;
            return -1;
            break;

        default:
            return 0;
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
