
#include <myserverApi.h>

int socketfd = 0, myerrno;
char __sockname[UNIX_PATH_MAX];

extern int foundP;

int readFile(char * pathname, void ** buf, size_t * size){


    if(socketfd == 0){
        myerrno = E_NOT_CON;
        return -1;
    }
    
    if(pathname == NULL){ myerrno = E_INV_PTH; return -1;}

    char pth[MAX_PATH];
    strncpy(pth, pathname, MAX_PATH);
    pth[MAX_PATH-1] = '\0';
    int reqType = READ_F, resp = 0;

    if(write(socketfd, &reqType, sizeof(int)) != sizeof(int)){  // scrivo il tipo di richesta
        myerrno = errno;
        return -1;
    }

    if(write(socketfd, pth, MAX_PATH) != MAX_PATH){  // scrivo il path
        myerrno = errno;
        return -1;
    }

    if(read(socketfd, &resp, sizeof(int)) != sizeof(int)){  // leggo la risposta
        myerrno = errno;
        return -1;
    }

    if(resp != 1){
        myerrno = errno;
        return -1;
    }

    int i_size;
    char * i_buf;

    if(read(socketfd, &i_size, sizeof(int)) != sizeof(int)){  // leggo la size
        myerrno = errno;
        return -1;
    }

    EXIT_ON(i_buf = malloc(i_size), == NULL); // alloco il buffer

    if(read(socketfd, i_buf, i_size) != i_size){  // leggo il contenuto
        free(i_buf);
        myerrno = errno;
        return -1;
    }

    if(read(socketfd, &resp, sizeof(int)) != sizeof(int)){  // leggo la risposta
        free(i_buf);
        myerrno = errno;
        return -1;
    }

    if(resp != 0){
        free(i_buf);
        myerrno = resp;
        return -1;
    }
    else {
        *buf = i_buf;
        *size = i_size;
        return 0;
    }
}

int removeFile(char * pathname){

    if(socketfd == 0){
        myerrno = E_NOT_CON;
        return -1;
    }

    if(pathname == NULL){ myerrno = E_INV_PTH; return -1;}

    char pth[MAX_PATH];
    strncpy(pth, pathname, MAX_PATH);
    pth[MAX_PATH-1] = '\0';
    int reqType = REMOVE_F, resp = 0;

    if(write(socketfd, &reqType, sizeof(int)) != sizeof(int)){  // scrivo il tipo di richesta
        myerrno = errno;
        return -1;
    }

    if(write(socketfd, pth, MAX_PATH) != MAX_PATH){  // scrivo il path
        myerrno = errno;
        return -1;
    }

    if(read(socketfd, &resp, sizeof(int)) != sizeof(int)){  // leggo la risposta
        myerrno = errno;
        return -1;
    }

    if(resp == 0){
        return 0;
    }
    else{
        myerrno = resp;
        return -1;
    }

}

int closeFile(char * pathname){

    if(socketfd == 0){
        myerrno = E_NOT_CON;
        return -1;
    }

    if(pathname == NULL){ myerrno = E_INV_PTH; return -1;}

    char pth[MAX_PATH];
    strncpy(pth, pathname, MAX_PATH);
    pth[MAX_PATH-1] = '\0';
    int reqType = CLOSE_F, resp = 0;

    if(write(socketfd, &reqType, sizeof(int)) != sizeof(int)){  // scrivo il tipo di richesta
        myerrno = errno;
        return -1;
    }

    if(write(socketfd, pth, MAX_PATH) != MAX_PATH){  // scrivo il path
        myerrno = errno;
        return -1;
    }

    if(read(socketfd, &resp, sizeof(int)) != sizeof(int)){  // leggo la risposta
        myerrno = errno;
        return -1;
    }

    if(resp == 0){
        return 0;
    }
    else{
        myerrno = resp;
        return -1;
    }

}

int unlockFile(char * pathname){

    if(socketfd == 0){
        myerrno = E_NOT_CON;
        return -1;
    }

    if(pathname == NULL){ myerrno = E_INV_PTH; return -1;}

    char pth[MAX_PATH];
    strncpy(pth, pathname, MAX_PATH);
    pth[MAX_PATH-1] = '\0';
    int reqType = UNLOCK_F, resp = 0;

    if(write(socketfd, &reqType, sizeof(int)) != sizeof(int)){  // scrivo il tipo di richesta
        myerrno = errno;
        return -1;
    }

    if(write(socketfd, pth, MAX_PATH) != MAX_PATH){  // scrivo il path
        myerrno = errno;
        return -1;
    }

    if(read(socketfd, &resp, sizeof(int)) != sizeof(int)){  // leggo la risposta
        myerrno = errno;
        return -1;
    }

    if(resp == 0){
        return 0;
    }
    else{
        myerrno = resp;
        return -1;
    }

}

int lockFile(char * pathname){

    if(socketfd == 0){
        myerrno = E_NOT_CON;
        return -1;
    }

    if(pathname == NULL){ myerrno = E_INV_PTH; return -1;}

    char pth[MAX_PATH];
    strncpy(pth, pathname, MAX_PATH);
    pth[MAX_PATH-1] = '\0';
    int reqType = LOCK_F, resp = 0;

    if(write(socketfd, &reqType, sizeof(int)) != sizeof(int)){  // scrivo il tipo di richesta
        myerrno = errno;
        return -1;
    }

    if(write(socketfd, pth, MAX_PATH) != MAX_PATH){  // scrivo il path
        myerrno = errno;
        return -1;
    }

    if(read(socketfd, &resp, sizeof(int)) != sizeof(int)){  // leggo la risposta
        myerrno = errno;
        return -1;
    }

    if(resp == 0){
        return 0;
    }
    else{
        myerrno = resp;
        return -1;
    }

}

int appendToFile(char * pathname, void * buf, size_t size, char * dirname){

    if(socketfd == 0){
        myerrno = E_NOT_CON;
        return -1;
    }
    
    if(pathname == NULL){ myerrno = E_INV_PTH; return -1;}
    if(buf == NULL || size < 1){ myerrno = E_BAD_RQ; return -1;}

    char pth[MAX_PATH];
    strncpy(pth, pathname, MAX_PATH);
    pth[MAX_PATH-1] = '\0';
    int reqType = APPEND_T_F, resp = 0;

    if(write(socketfd, &reqType, sizeof(int)) != sizeof(int)){  // scrivo il tipo di richesta
        myerrno = errno;
        return -1;
    }

    if(write(socketfd, &size, sizeof(int)) != sizeof(int)){  // scrivo la size
        myerrno = errno;
        return -1;
    }
    
    if(write(socketfd, pth, MAX_PATH) != MAX_PATH){  // scrivo il path
        myerrno = errno;
        return -1;
    }
    
    if(write(socketfd, buf, size) != size){  // scrivo il buffer
        myerrno = errno;
        return -1;
    }
 
    if(read(socketfd, &resp, sizeof(int)) != sizeof(int)){  // leggo la risposta
        myerrno = errno;
        return -1;
    }

    if(resp == 0){
        return 0;
    }

    if(resp != 1){
        myerrno = resp;
        return -1;
    }

    // lettura file di risposta
    
    if(resp == 1){ // devo leggere N file: size path cont
        FILE* outFile;
        while(1){

            if(read(socketfd, &size, sizeof(int)) != sizeof(int)){ myerrno = errno; return -1;} // leggo la size, se è -1 ho finito
            if(size == -1){
                break;
            }
            
            if(read(socketfd, pth, MAX_PATH) != MAX_PATH){ myerrno = errno; return -1;} // leggo il path

            EXIT_ON(buf = malloc(size), == NULL); // alloco il buffer
            
            if(read(socketfd, buf, size) != size){ myerrno = errno; free(buf); return -1;} // leggo il contenuto

            if (( outFile = fopen(pth, "wb")) == NULL){
                myerrno = errno;
            }
            
            if(fwrite(buf, 1, size, outFile) != size){  // scrivo contenuto su file
                myerrno = errno;
                free(buf);
                fclose(outFile);
                return -1;
            }

            free(buf);
            fclose(outFile);
        }
    }
    
    if(read(socketfd, &resp, sizeof(int)) != sizeof(int)){ myerrno = errno; return -1;}

    if(resp == 0){
        return 0;
    }
    else{
        myerrno = resp;
        return -1;
    }
    
}

int writeFile(char* pathname, char * dirname){

    if(socketfd == 0){
        myerrno = E_NOT_CON;
        return -1;
    }

    if(pathname == NULL){
        myerrno = E_INV_PTH;
        return -1;
    }

    char pth[MAX_PATH], *buf;
    strncpy(pth, pathname, MAX_PATH);
    pth[MAX_PATH-1] = '\0';
    int reqType = WRITE_F, resp = 0;
    FILE * inFile;

    if((inFile = fopen(pth, "rb")) == NULL){
        myerrno = errno;
        fprintf(stderr,"impossibile aprire file\n");
        return -1;
    }
 
    // leggo il file
    size_t size = 0;

    fseek(inFile, 0, SEEK_END);
    size = ftell(inFile);
    fseek(inFile, 0, SEEK_SET);
    EXIT_ON(buf = malloc(size), == NULL);
    if( fread(buf, 1, size, inFile) != size){
        fprintf(stderr, "impossibile leggere il file\n");
        myerrno = errno;
        free(buf);
        return -1;
    }
    
    fclose(inFile);

    if(write(socketfd, &reqType, sizeof(int)) != sizeof(int)){  // scrivo il tipo di richesta
        myerrno = errno;
        return -1;
    }
    if(write(socketfd, &size, sizeof(int)) != sizeof(int)){  // scrivo la size
        myerrno = errno;
        return -1;
    }
    if(write(socketfd, pth, MAX_PATH) != MAX_PATH){  // scrivo il path
        myerrno = errno;
        return -1;
    }
    if(write(socketfd, buf, size) != size){  // scrivo il contenuto
        myerrno = errno;
        return -1;
    }
    free(buf);

    if(read(socketfd, &resp, sizeof(int)) != sizeof(int)){  // leggo la risposta
        myerrno = errno;
        return -1;
    }
    if(resp == 0){
        return 0;
    }

    if(resp != 1){
        myerrno = resp;
        return -1;
    }
    // lettura file di risposta
    
    if(resp == 1){ // devo leggere N file: size path cont
        FILE* outFile;
        while(1){

            if(read(socketfd, &size, sizeof(int)) != sizeof(int)){ myerrno = errno; return -1;} // leggo la size, se è -1 ho finito
            if(size == -1){
                break;
            }
            
            if(read(socketfd, pth, MAX_PATH) != MAX_PATH){ myerrno = errno; return -1;} // leggo il path

            EXIT_ON(buf = malloc(size), == NULL); // alloco il buffer
            
            if(read(socketfd, buf, size) != size){ myerrno = errno; free(buf); return -1;} // leggo il contenuto

            if (( outFile = fopen(pth, "wb")) == NULL){
                perror("open");
                myerrno = errno;
                fprintf(stderr,"impossibile aprire o creare file\n");
            }
            
            if(fwrite(buf, 1, size, outFile) != size){  // scrivo contenuto su file
                myerrno = errno;
                free(buf);
                fclose(outFile);
                return -1;
            }

            free(buf);
            fclose(outFile);

        }
    }

    if(read(socketfd, &resp, sizeof(int)) != sizeof(int)){  // leggo la risposta
        myerrno = errno;
        return -1;
    }

    if(resp == 0){
        return 0;
    }
    else{
        myerrno = resp;
        return -1;
    }

}

int openFile(const char* pathname, int flags){

    if(socketfd == 0){
        myerrno = E_NOT_CON;
        return -1;
    }
    
    if(flags < 0 || flags > 3) {
        myerrno = E_INV_FLG;
        return -1;
    }

    char pth[MAX_PATH];
    strncpy(pth, pathname, MAX_PATH);
    pth[MAX_PATH-1] = '\0';
    int reqType = OPEN_F;
    int resp = 0;

    if(pth == NULL){
        myerrno = E_INV_PTH;
        return -1;
    }

    if(write(socketfd, &reqType, sizeof(int)) != sizeof(int)){  // scrivo il tipo di richesta
        myerrno = errno;
        return -1;
    }

    if(write(socketfd, &pth, MAX_PATH) != MAX_PATH){  // scrivo il path
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

    if(resp == 0){
        return 0;
    }
    else{
        myerrno = resp;
        return -1;
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

    if(socketfd == 0){
        myerrno = E_NOT_CON;
        return -1;
    }

    if (sockname == NULL || (strncmp(__sockname, sockname, UNIX_PATH_MAX) != 0)) {
        myerrno = E_INV_SCK;
        return -1;
    }
    close(socketfd);
    socketfd = 0;
    return 0;
}

void myperror(const char * str){
    if(myerrno >= 140){
        fprintf(stderr, "errore %d: ", myerrno);
        fprintf(stderr, "%s\n", str);
    }
    else{
        perror(str);
    }
}
