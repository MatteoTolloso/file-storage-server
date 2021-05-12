
#include <parser.h>


int parse(char ** socket_name, int * max_num_file, int * max_dim_storage, int * num_thread_worker){

    FILE * fptr;
    EXIT_ON( fptr = fopen(CONFIG_FILE_PATH, "r"), ==  NULL);
    
    #ifdef DEBUG
    printf("success open file \n");
    #endif

    char * key, * value;

    EXIT_ON( key = malloc(PARSE_BUF_DIM), ==   NULL);
    EXIT_ON( value = malloc(PARSE_BUF_DIM), ==  NULL);

    while ( (memset(key,'\0', PARSE_BUF_DIM) != NULL) && (fscanf(fptr, "%[^=]", key) == 1 )){

        fgetc(fptr); // consuma il carattere '='
        key[strnlen(key, PARSE_BUF_DIM)] = '\0';
        EXIT_ON(memset(value, '\0', PARSE_BUF_DIM), == NULL);
        if (fscanf(fptr, "%[^;]", value) != 1 ) break;
        fgetc(fptr);  // consuma il carattere ';'
        value[strnlen(value, PARSE_BUF_DIM)] = '\0';
        (fgetc(fptr)); // consuma il carattere '\n' (se non è presente prenderò EOF)

        #ifdef DEBUG
        printf("key:%s: valore:%s:\n", key, value);
        #endif

        // gestisco i vari casi

        if (strncmp(key, ARG_1, strnlen(ARG_1, PARSE_BUF_DIM)) == 0){ // gestisco socket_name  
            EXIT_ON(*socket_name = malloc(PARSE_BUF_DIM), == NULL);
            EXIT_ON(memset(*socket_name, '\0', PARSE_BUF_DIM), == NULL);
            EXIT_ON(strncpy(*socket_name, value, PARSE_BUF_DIM), == NULL);
            continue;
        }

        if (strncmp(key, ARG_2, strnlen(ARG_2, PARSE_BUF_DIM)) == 0){ // gestisco max_num_file
            *max_num_file = atoi(value);
            continue;
        }

        if (strncmp(key, ARG_3, strnlen(ARG_3, PARSE_BUF_DIM)) == 0){ // gestisco max_dim_storage
            *max_dim_storage = atoi(value);
            continue;
        }

        if (strncmp(key, ARG_4, strnlen(ARG_4, PARSE_BUF_DIM)) == 0){ // gestisco num_thread_worker 
            *num_thread_worker = atoi(value);
            continue;
        }

        printf("parametro di configurazione non riconosciuto\n");
        return -1;
        
    }

    if(feof(fptr)){    // uscito perchè fine del file 
        free(key);
        free(value);
        #ifdef DEBUG
        printf("parsing terminato correttamente\n");
        #endif
        return 0;
    }
    else{
        perror("lettura config.txt");
        return -1;
    }


}

