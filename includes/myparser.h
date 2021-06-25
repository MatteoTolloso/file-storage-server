#ifndef _MY_PARSE_H
#define _MY_PARSE_H

#define _POSIX_C_SOURCE 200809L 
#include <stdlib.h>
#include <stdio.h>
#include <myutil.h>

#include <string.h>

#define PARSE_BUF_DIM 64


#define ARG_1 "socket_name"
#define ARG_2 "max_num_file"
#define ARG_3 "max_dim_storage"
#define ARG_4 "num_thread_worker"
#define ARG_5 "log_file_path"
/*
per aggiungere un altro parametro:
- aggiungere una define con il nome del parametro
- aggiungere alla firma della funzione parse
- aggiungere nel corpo di parse un blocco per gestire quella determinata coppia <key, value>

*/

int parse(char* configpath,char**  logpath, char ** socket_name, int * max_num_file, int * max_dim_storage, int * num_thread_worker);

#endif /* _MY_PARSE_H */