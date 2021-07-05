#include <myhandler.h>



void ter_handler(int sig){
    int n;
    if ((sig == SIGINT) || (sig == SIGQUIT)){
        n=2;
        if(write(pipeSigWriting, &n, sizeof(int)) != sizeof(int)){
            perror("write");
        }
        return;
    }
    if( sig == SIGHUP){
        n=1;
        if(write(pipeSigWriting, &n, sizeof(int)) != sizeof(int)){
            perror("write");
        }
        return;
    }

    EXIT_ON(write(1, "unknown signal", 15), <= 0);
}

void print_handler(int sig){
    EXIT_ON(write(2, "segnale ricevuto dal worker: %d", sig), == 0);
    return;
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

    EXIT_ON(pthread_sigmask(SIG_SETMASK, &complementar, NULL), != 0);
}

void worker_handler_installer(){

    sigset_t fullmask;
    sigfillset(&fullmask);
    EXIT_ON(pthread_sigmask(SIG_SETMASK, &fullmask, NULL), != 0);

    
    struct sigaction sa;
    EXIT_ON(memset(&sa, 0, sizeof(struct sigaction)), == NULL);
    
    sa.sa_handler = print_handler;
    EXIT_ON( sigaction(SIGPIPE, &sa, NULL), != 0);


}

