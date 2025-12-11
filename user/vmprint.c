#include "kernel/types.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
    vmprint();   // 系统调用
    exit(0);
}