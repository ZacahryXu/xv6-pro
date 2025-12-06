//
// Created by ZacharyXu on 2025/12/5.
//

#include "kernel/types.h"
#include "user/user.h"
int main(int argc,const char *argv[])
{
    if (argc!=2)
    {
        fprintf(2, "休眠时间不能为空！\n");
        exit(1);
    }
    sleep(atoi(argv[1]));
    exit(0);
}