#include <myconnection.h>

int updatemax(fd_set set, int fdmax) {
    for(int i=(fdmax-1);i>=0;--i)
	if (FD_ISSET(i, &set)) return i;
    EXIT_ON("error in updatemax", == NULL);
    return -1;
}

int init_server(char * sck_name){
    EXIT_ON(strnlen(sck_name, UNIX_PATH_MAX) >= UNIX_PATH_MAX - 2, != 0);
    sck_name[strnlen(sck_name, UNIX_PATH_MAX)] = '\0';
    int socket_fd;
    EXIT_ON(socket_fd = socket(AF_UNIX, SOCK_STREAM, 0), == -1);
    struct sockaddr_un server_addr;
    EXIT_ON(memset(&server_addr, 0, sizeof(struct sockaddr_un)), == NULL);
    server_addr.sun_family = AF_UNIX;
    EXIT_ON(strncpy(server_addr.sun_path, sck_name, UNIX_PATH_MAX-1),  == NULL);
    EXIT_ON(bind(socket_fd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_un)), != 0);
    EXIT_ON(listen(socket_fd, MAX_CONNECTION_QUEUE), != 0);
    return socket_fd; 
}