#include <myserverApi.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <time.h>

//lo deve prendere da riga di comando 
#define SOCKNAME "./mysock"

#define HELP "stringa di aiuto"

extern int socketfd, myerrno;
extern char __sockname[UNIX_PATH_MAX];
char returnDir[MAX_PATH] = "";
char readDir[MAX_PATH] = "";
int delay = 0, foundp = 0, r;

void listfile(const char nomedir[], int * n);
int isdot(const char dir[]) ;
int is_comand(char * str);
char to_comand(char * str);
void send_dir(char * str);
void send_file(char str[]);
void read_file(char * str);
void read_n_file(char * str);
int msleep(long msec);
void client_log(char * format, ... );

int main(int argc, char ** argv){

    if(argc == 1){return 0;}

    int foundf = 0, i = 1, foundW = 0,  
        foundD = 0, foundr = 0, foundw = 0, foundd = 0, foundR =0;
    char comand;

    while (i < argc){   // primo ciclo di controllo
        if(is_comand(argv[i])){
            comand = to_comand(argv[i]);
            switch (comand)
            {
            case 'h':;{
                printf(HELP); 
                return 0;
                break;
            }
            case 'p':;{
                foundp++;
                break;
            }
            case 'f':;{
                foundf++;
                if( argv[i+1] == NULL || is_comand(argv[i+1])){
                    fprintf(stderr, "opzione -f non usata correttamente\n");
                    return 0;
                }
                break;
            }
            case 'w':;{
                foundw = 1;
                if( argv[i+1] == NULL || is_comand(argv[i+1])){
                    fprintf(stderr, "opzione -w non usata correttamente\n");
                    return 0;
                }
                break;
            }
            case 'W':;{
                foundW = 1;
                if( argv[i+1] == NULL || is_comand(argv[i+1])){
                    fprintf(stderr, "opzione -W non usata correttamente\n");
                    return 0;
                }
                break;
            }
            case 'D':;{ 
                foundD = 1;
                if( argv[i+1] == NULL || is_comand(argv[i+1])){
                    fprintf(stderr, "opzione -D non usata correttamente\n");
                    return 0;
                }
                break;
            }
            case 'r':;{
                foundr = 1;
                if( argv[i+1] == NULL || is_comand(argv[i+1])){
                    fprintf(stderr, "opzione -r non usata correttamente\n");
                    return 0;
                }
                break;
            }
            case 'd':;{
                foundd = 1;
                if( argv[i+1] == NULL || is_comand(argv[i+1])){
                    fprintf(stderr, "opzione -d non usata correttamente\n");
                    return 0;
                }
                break;
            }
            case 'R':;{
                foundR = 1;
                break;
            }
            case 'l':;{
                if( argv[i+1] == NULL || is_comand(argv[i+1])){
                    fprintf(stderr, "opzione -l non usata correttamente\n");
                    return 0;
                }
                break;
            }
            case 'u':;{
                if( argv[i+1] == NULL || is_comand(argv[i+1])){
                    fprintf(stderr, "opzione -u non usata correttamente\n");
                    return 0;
                }
                break;
            }
            case 'c':;{
                if( argv[i+1] == NULL || is_comand(argv[i+1])){
                    fprintf(stderr, "opzione -c non usata correttamente\n");
                    return 0;
                }
                break;
            }
            case 't':;{
                if( argv[i+1] == NULL || is_comand(argv[i+1])){
                    fprintf(stderr, "opzione -t non usata correttamente\n");
                    return 0;
                }
                delay = atoi(argv[i+1]);
                break;
            }
            }
        }
        i++;
    }

    if(foundD == 1 && (foundW + foundw) <= 0){
        fprintf(stderr, "opzione -D non usata correttamente\n");
        return 0;
    }

    if(foundd == 1 && (foundR + foundr) <= 0){
        fprintf(stderr, "opzione -d non usata correttamente\n");
        return 0;
    }

    if(foundp > 1){
        fprintf(stderr, "opzione -p non usata correttamente\n");
        return 0;
    }
    
    if(foundf != 1){
        fprintf(stderr, "opzione -f non usata correttamente\n");
        return 0;
    }
    if(delay < 0){
        fprintf(stderr, "opzione -t non usata correttamente\n");
        return 0;
    }
    
    
    i = 1;
    while (i < argc){
        
        comand = to_comand(argv[i]);
        switch (comand)
        {
        
            case 'f':;{  
                PIE(openConnection(argv[i+1]));
                client_log("Apro la connessione con il socket: %s", argv[i+1]);
                i++; // devo saltare un ulteriore parametro
                msleep(delay);
                break;
            }

            case 'w':;{   
                send_dir(argv[i+1]);
                i++;
                msleep(delay);
                break;
            }

            case 'W':;{
                send_file(argv[i+1]);
                i++;
                msleep(delay);
                break;
            }

            case 'D':;{ // testare questa opzione
                strcpy(returnDir, argv[i+1]);
                client_log("Cartella di salvataggio della cache aggiornata in: %s", returnDir);
                i++;
                break;
            }

            case 'r':;{
                read_file(argv[i+1]);
                i++;
                msleep(delay);
                break;
            }
            case 'd':;{
                strcpy(readDir, argv[i+1]);
                client_log("Cartella di lettura aggiornata in: %s", readDir);
                break;
            }
            case 'R':;{
                read_n_file(argv[i+1]);
                msleep(delay);
                break;
            }
            case 'l':;{
                lock_file(argv[i+1]);
                i++;
                msleep(delay);
                break;
            }
            case 'u':;{
                unlock_file(argv[i+1]);
                i++;
                msleep(delay);
                break;
            }
            case 'c':;{
                remove_file(argv[i+1]);
                i++;
                msleep(delay);
                break;
            }
        }   
            
        i++;
    }
    
    return 0;

}
void remove_file(char * str){

    char file[MAX_PATH];
    int pos =0;

    while(1){
        
        if(str[pos] == '\0'){
            strncpy(file, str, pos +1);
            PIE(r = openFile(file, 0));
            PIE(r = removeFile(file));
            client_log("Rimosso il file %s con esito %d", file, r);
            return;
        }
        else if(str[pos] == ','){
            str[pos] = '\0';
            strncpy(file, str, pos +1);
            PIE(r = openFile(file, 0));
            PIE(r = removeFile(file));
            client_log("Rimosso il file %s con esito %d", file, r);
            str = str + pos + 1;
            pos = 0;
        }
        else{
            pos++;
        }
    }

}
void unlock_file(char * str){

    char file[MAX_PATH];
    int pos =0;

    while(1){
        
        if(str[pos] == '\0'){
            strncpy(file, str, pos +1);
            PIE(r = openFile(file, 0));
            PIE(r = unlockFile(file));
            client_log("Unlock del file %s con esito %d", file, r);
            return;
        }
        else if(str[pos] == ','){
            str[pos] = '\0';
            strncpy(file, str, pos +1);
            PIE(r = openFile(file, 0));
            PIE(r = unlockFile(file));
            client_log("Unlock del file %s con esito %d", file, r);
            str = str + pos + 1;
            pos = 0;
        }
        else{
            pos++;
        }
    }

}

void lock_file(char * str){

    char file[MAX_PATH];
    int pos =0;

    while(1){
        
        if(str[pos] == '\0'){
            strncpy(file, str, pos +1);
            PIE(r = openFile(file, 0));
            PIE(r = lockFile(file));
            client_log("Lock del file %s con esito %d", file, r);
            return;
        }
        else if(str[pos] == ','){
            str[pos] = '\0';
            strncpy(file, str, pos +1);
            PIE(r = openFile(file, 0));
            PIE(r = lockFile(file));
            client_log("Lock del file %s con esito %d", file, r);
            str = str + pos + 1;
            pos = 0;
        }
        else{
            pos++;
        }
    }

}

void read_n_file(char * str){

    int n;
    if(str == NULL || is_comand(str)) n=0; // devo leggere tutti i file
    else{
        str += 2;
        n = atoi(str);
    }

    if(strcmp(readDir, "") == 0){
        fprintf(stderr, "retDir non inizializzata\n");
        return;
    }

    PIE(readNFiles(n, readDir));
    client_log("Lettura di %d files con esito %d", n, r);

}

void read_file(char * str){
    
    char returnPath[MAX_PATH], file[MAX_PATH];
    int pos =0;

    while(1){
        void * buf = NULL; size_t size; FILE * fileptr;
        int r;
        
        if(str[pos] == '\0'){
            strncpy(file, str, pos +1);
            
            PIE(r = readFile(file, &buf, &size)); // legge dal server
            client_log("Lettura del file %s di %d byte con esito %d", file,size, r);
            if(r == -1) {return;}

            if(strcmp(readDir, "") != 0){   // se ho specificato una cartella in cui salvare i file letti
                strcpy(returnPath, readDir);
                strcat(returnPath, (file+onlyName(file)));
                EXIT_ON( fileptr = fopen(returnPath, "wb"), == NULL);
                EXIT_ON(fwrite(buf, 1, size, fileptr), != size);
                client_log("Salvataggio del file %s nella cartella %s", file + onlyName(file), readDir);
                free(buf);
            }

            return;
        }
        else if(str[pos] == ','){
            str[pos] = '\0';
            strncpy(file, str, pos +1);
            
            PIE(r = readFile(file, &buf, &size)); // legge dal server
            client_log("Lettura del file %s di %d byte con esito %d", file,size, r);

            if(strcmp(readDir, "") != 0){
                strcpy(returnPath, readDir);
                strcat(returnPath, (file+onlyName(file)));
                EXIT_ON( fileptr = fopen(returnPath, "wb"), == NULL);
                EXIT_ON(fwrite(buf, 1, size, fileptr), != size);
                client_log("Salvataggio del file %s nella cartella %s", file + onlyName(file), readDir);
                free(buf);
            }

            str = str + pos + 1;
            pos = 0;
        }
        else{
            pos++;
        }
    }
}


void send_file(char str[]){ 

    char file[MAX_PATH];
    int pos =0;

    while(1){
        
        if(str[pos] == '\0'){
            strncpy(file, str, pos +1);
            PIE(r = openFile(file, O_CREATE | O_LOCK));
            client_log("Apertura del file %s con flag %d, esito %d", file, O_CREATE | O_LOCK, r);
            if(strcmp(returnDir, "") == 0){ // cartella di ritorno non specificata
                PIE(r = writeFile(file, NULL));
                client_log("Scrittura del file %s nel server senza salvare i file evicted, esito: %d", file, r);
            }
            else{
                PIE(r = writeFile(file, returnDir));
                client_log("Scrittura del file %s nel server con salvataggio dei file evicted in %s, esito: %d", file, returnDir, r);
            }
            
            return;
        }
        else if(str[pos] == ','){
            str[pos] = '\0';
            strncpy(file, str, pos +1);
            PIE(openFile(file, O_CREATE | O_LOCK));
            if(strcmp(returnDir, "") == 0){ // cartella di ritorno non specificata
                PIE(r = writeFile(file, NULL));
                client_log("Scrittura del file %s nel server senza salvare i file evicted, esito: %d", file, r);
            }
            else{
                PIE(r = writeFile(file, returnDir));
                client_log("Scrittura del file %s nel server con salvataggio dei file evicted in %s, esito: %d", file, returnDir, r);
            }
            str = str + pos + 1;
            pos = 0;
        }
        else{
            pos++;
        }
    }

}

void listfile(const char nomedir[], int * n) {
    // controllo che il parametro sia una directory
    struct stat statbuf;
    
    EXIT_ON(stat(nomedir,&statbuf),!= 0);

    DIR * dir;
    fprintf(stdout, "Directory %s:\n",nomedir);
    
    if ((dir=opendir(nomedir)) == NULL) {
        perror("opendir");
        printf("Errore aprendo la directory %s\n", nomedir);
        return;
    } 
    
    struct dirent *file;
    
    while(((errno=0, file = readdir(dir)) != NULL) && *n != 0 ) {
        struct stat statbuf;
        char filename[MAX_PATH]; 
        int len1 = strlen(nomedir);
        int len2 = strlen(file->d_name);
        if ((len1+len2+2)>MAX_PATH) {
            fprintf(stderr, "ERRORE: MAXFILENAME troppo piccolo\n");
            exit(EXIT_FAILURE);
        }	    
        strncpy(filename,nomedir,      MAX_PATH-1);
        strncat(filename,"/",          MAX_PATH-1);
        strncat(filename,file->d_name, MAX_PATH-1);
        
        if (stat(filename, &statbuf)==-1) {
            perror("eseguendo la stat");
            printf("Errore nel file %s\n", filename);
            return;
        }
        if(S_ISDIR(statbuf.st_mode)) {
            if ( !isdot(filename) ) 
                listfile(filename, n);
        } 
        else {

            (*n)--;
            PIE(openFile(filename, O_CREATE | O_LOCK));   // creo il file nel server
            if(strcmp(returnDir, "") == 0){ // cartella di ritorno non specificata
                PIE(r = writeFile(filename, NULL));
                client_log("Scrittura del file %s nel server senza salvare i file evicted, esito: %d", filename, r);
            }
            else{
                PIE(r = writeFile(filename, returnDir));
                client_log("Scrittura del file %s nel server con salvataggio dei file evicted in %s, esito: %d", filename, returnDir, r);
            }
        }
        
    }
    if (errno != 0) perror("readdir");
    closedir(dir);
    
}

int isdot(const char dir[]) {
  int l = strlen(dir);
  
  if ( (l>0 && dir[l-1] == '.') ) return 1;
  return 0;
}

int is_comand(char * str){
    if((str != NULL) && (str[0] == '-') && ((str + 1) != NULL)  && (str[2] == '\0') ){
        return 1;
    }
    return 0;
}

char to_comand(char * str){
    return str[1];
}

void send_dir(char * str){

    if(strlen(str) > MAX_PATH -1){printf("error"); return;}

    int n = 0, virpos = 0;
    char dirname[MAX_PATH]; 
    
    while (str[virpos] != '\0' && str[virpos] != ','){ // finche arrivo alla fine o ad una virgola
        virpos++;
    }
    strncpy(dirname,str, virpos);
    dirname[virpos] = '\0'; // dirname è ok
    if(str[virpos] == ','){    //se sono arrivato alla virgola
        str = str + virpos + 3;    // salto n=
        if(str != NULL){ 
            n = atoi(str);
        }
        else{
            printf("error in number of files");
            printf(HELP);
            return;
        }
    }

    if (n<=0) n=-1;

    // navigare i file nelle cartelle e inviarli al server

    // controllo che l'argomaneto sia una directory
    struct stat statbuf;
    if (stat(dirname,&statbuf)!= 0 || !S_ISDIR(statbuf.st_mode)){
	    fprintf(stderr, " \"%s\" non e' una directory\n", dirname);
        printf(HELP);
        return;
    }    

    listfile(dirname, &n);

}

int msleep(long msec){
    struct timespec ts;
    int res;

    if (msec < 0)
    {
        errno = EINVAL;
        return -1;
    }

    ts.tv_sec = msec / 1000;
    ts.tv_nsec = (msec % 1000) * 1000000;

    do {
        res = nanosleep(&ts, &ts);
    } while (res && errno == EINTR);

    return res;
}

void client_log(char * format, ... ){
    if(foundp == 0) return;
    va_list arglist;
    va_start(arglist, format);
    vfprintf(stderr, format, arglist);
    fprintf(stderr, "\n");
    va_end(arglist);
}