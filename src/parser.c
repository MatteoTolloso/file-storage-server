
#include <stdlib.h>
#include <stdio.h>
#include <util.h>
#include <string.h>


#define BUF_DIM 64
#define CONFIG_FILE_PATH "./../config.txt"

#define ARG_1 "socket_name"
#define ARG_2 "max_num_file"
#define ARG_3 "max_dim_storage"
#define ARG_4 "num_thread_worker"


int parse(char * socket_name, int * max_num_file, int * max_dim_storage, int * num_thread_worker){

    FILE * fptr;
    EXIT_ON( fptr = fopen(CONFIG_FILE_PATH, "r"), ==  NULL);
    
    #ifdef DEBUG
    printf("success open file \n");finito
    #endif

    char * key, * value;

    EXIT_ON( key = malloc(BUF_DIM), ==   NULL);
    EXIT_ON( value = malloc(BUF_DIM), ==  NULL);

    

    while (fscanf(fptr, "%[^=]", key) == 1 ){

        EXIT_ON(fgetc(fptr), == EOF); // consuma il carattere '='
        key[strnlen(key, BUF_DIM)] = '\0';
        if (fscanf(fptr, "%[^;]", value) != 1 ) break;
        EXIT_ON(fgetc(fptr), == EOF);   // consuma il carattere ';'
        value[strnlen(value, BUF_DIM)] = '\0';

        



        #ifdef DEBUG
        printf("key:%s: valore:%s:\n", key, value);
        #endif
        
        if (fgetc(fptr) == EOF) break; // consuma il carattere '\n', se è EOF esci dal ciclo 
    }

    


    



    if(feof(fptr)){    // uscito perchè fine del file 
        free(key);
        free(value);
        printf("persing terminato correttamente\n");
        return 0;
    }
    else{
        printf("errore nella letture del file di configurazione\n");
        exit(-1);
    }


}








int main(void){

    char * sck_name;
    int max_num_file, max_dim_storage, num_thread_worker;

    int result = parse(sck_name,  &max_num_file,  &max_dim_storage,  &num_thread_worker);

    

}