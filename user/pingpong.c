//
// Created by ZacharyXu on 2025/12/8.
//
#include "kernel/types.h"
#include "user/user.h"
int main(int argc,char *argv[])
{
    int p[2];
    pipe(p);
    int pid  = fork();
    char P = 'P';
    if (pid==0)
    {
        int pid = getpid();
        printf("<%d>:received ping",pid);
        close(0);
        dup(p[0]);
        close(p[0]);
        write(0,&P,1);
        close(p[1]);
        exit(0);
    }else
    {
        int pid = getpid();
        printf("<%d>:received pong",pid);
        read(0,&P,1);
        close(p[0]);
        close(p[1]);
        exit(0);
    }
}