#include <myhandler.h>



void ter_handler(int sig){
    int n;
    if ((sig == SIGINT) || (sig == SIGQUIT)){
        n=2;
        if(write(pipeSigWriting, &n, sizeof(int)) != sizeof(int)){
            perror("write");
        }
        printf("HANDLER: ho scritto sulla pipe dei segnali %d\n", n); // da rimuovere, non sicuro
        return;
    }
    if( sig == SIGHUP){
        n=1;
        if(write(pipeSigWriting, &n, sizeof(int)) != sizeof(int)){
            perror("write");
        }
        printf("HANDLER: ho scritto sulla pipe dei segnali %d\n", n);   // da rimuovere
        return;
    }

    EXIT_ON(write(1, "unknown signal", 15), <= 0);
}

void handler_installer(){

    sigset_t fullmask, handlermask, complementar;
    sigfillset(&fullmask);
    sigemptyset(&handlermask);
    sigfillset(&complementar);
    EXIT_ON(pthread_sigmask(SIG_SETMASK, &fullmask, NULL), != 0);

    sigaddset(&handlermask, SIGINT);
    sigaddset(&handlermask, SIGQUIT);
    sigaddset(&handlermask, SIGHUP);
    sigdelset(&complementar, SIGINT);
    sigdelset(&complementar, SIGQUIT);
    sigdelset(&complementar, SIGHUP);
    
    struct sigaction sa;
    EXIT_ON(memset(&sa, 0, sizeof(struct sigaction)), == NULL);
    
    sa.sa_handler = ter_handler;
    sa.sa_mask = handlermask;
    EXIT_ON( sigaction(SIGINT, &sa, NULL), != 0);
    EXIT_ON( sigaction(SIGQUIT, &sa, NULL), != 0);
    EXIT_ON( sigaction(SIGHUP, &sa, NULL), != 0);

    sigemptyset(&complementar); // da rimuovere
    EXIT_ON(pthread_sigmask(SIG_SETMASK, &complementar, NULL), != 0);


}