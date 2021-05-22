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
#define SOCKNAME "./mysock"

int main(int argc, char ** argv){

    struct sockaddr_un server_addr;
    int socketfd;
    socketfd = socket(AF_UNIX, SOCK_STREAM, 0);
    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, SOCKNAME, strlen(SOCKNAME) +1);

    connect(socketfd, (struct sockaddr *)&server_addr, sizeof(server_addr));

    int n = 3;
    write(socketfd, &n, sizeof(int));
    sleep(3);

    //close(socketfd);

}