#include <myserverApi.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>

//lo deve prendere da riga di comando 
#define SOCKNAME "./mysock"

#define HELP "stringa di aiuto"

extern int socketfd, myerrno;
extern char __sockname[UNIX_PATH_MAX];
char returnDir[MAX_PATH] = "";
char readDir[MAX_PATH] = "";

void listfile(const char nomedir[], int * n);
int isdot(const char dir[]) ;
int is_comand(char * str);
char to_comand(char * str);
void send_dir(char * str);
void send_file(char str[]);
void read_file(char * str);

int main(int argc, char ** argv){

    if(argc == 1){return 0;}

    int foundf = 0, i = 1, foundW = 0, foundp = 0, 
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
            }
            case 'R':;{
                foundR = 1;
                if( argv[i+1] == NULL || is_comand(argv[i+1])){
                    fprintf(stderr, "opzione -R non usata correttamente\n");
                    return 0;
                }
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
    
    i = 1;
    while (i < argc){
        
        comand = to_comand(argv[i]);
        switch (comand)
        {
        
            case 'f':;{  
                PIE(openConnection(argv[i+1]));
                i++; // devo saltare un ulteriore parametro
                
                break;
            }

            case 'w':;{   
                send_dir(argv[i+1]);
                i++;
                break;

            }

            case 'W':;{
                send_file(argv[i+1]);
                i++;
                break;
            }

            case 'D':;{ // testare questa opzione
                strcpy(returnDir, argv[i+1]);
                i++;
                break;
            }

            case 'r':;{
                read_file(argv[i+1]);
                i++;
                
                break;
            }
            case 'd':;{
                strcpy(readDir, argv[i+1]);
                break;
            }

            case 'r':;{
                read_file(argv[i+1]);
                break;
            }
        }   
            
        i++;
    }
    
    return 0;

}

void read_file(char * str){
    
    char file[MAX_PATH];
    int pos =0;

    // spostarsi nella cartella dove mettere i file e scrivere il buffer della read nel file (creato)
    
    while(1){
        
        if(str[pos] == '\0'){
            strncpy(file, str, pos +1);
            PIE(openFile(file, O_CREATE | O_LOCK));
            PIE(writeFile(file, returnDir));
            return;
        }
        else if(str[pos] == ','){
            str[pos] = '\0';
            strncpy(file, str, pos +1);
            PIE(openFile(file, O_CREATE | O_LOCK));
            PIE(writeFile(file, returnDir));
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
            PIE(openFile(file, O_CREATE | O_LOCK));
            PIE(writeFile(file, returnDir));
            return;
        }
        else if(str[pos] == ','){
            str[pos] = '\0';
            strncpy(file, str, pos +1);
            PIE(openFile(file, O_CREATE | O_LOCK));
            PIE(writeFile(file, returnDir));
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
            PIE(writeFile(filename, returnDir)); // scrivo il file
        
            fprintf(stdout, "file: %s, %d\n", filename, *n);		
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