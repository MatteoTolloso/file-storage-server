#ifndef _CONNECTION_H
#define _CONNECTION_H
#define _POSIX_C_SOURCE 200809L 

#include <sys/socket.h>
#include <sys/un.h>
#include <util.h>
#include <sys/select.h>
#include <string.h>
#include <stdio.h>
#define UNIX_PATH_MAX 108
#define MAX_CONNECTION_QUEUE 32

int updatemax(fd_set set, int fdmax);

int init_server(char * sck_name);

#endif /* _CONNECTION_H */