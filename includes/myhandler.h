#ifndef _MY_HANDLER_H
#define _MY_HANDLER_H
#define _POSIX_C_SOURCE 200809L
#include <signal.h>
#include <myutil.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>


extern int pipeSigWriting;

void ter_handler(int sig);

void handler_installer();

#endif /* _MY_HANDLER_H */