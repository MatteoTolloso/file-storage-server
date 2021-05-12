#ifndef _PARSE_H
#define _PARSE_H

#include <stdlib.h>
#include <stdio.h>
#include <util.h>
#include <string.h>

#define PARSE_BUF_DIM 64
#define CONFIG_FILE_PATH "config.txt"

#define ARG_1 "socket_name"
#define ARG_2 "max_num_file"
#define ARG_3 "max_dim_storage"
#define ARG_4 "num_thread_worker"
/*
per aggiungere un altro parametro:
- aggiungere una define con il nome del parametro
- aggiungere alla firma della funzione parse
- aggiungere nel corpo di parse un blocco per gestire quella determinata coppia <key, value>

*/

int parse(char ** socket_name, int * max_num_file, int * max_dim_storage, int * num_thread_worker);

#endif /* _PARSE_H */