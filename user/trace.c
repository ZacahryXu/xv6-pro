// user/trace.c
#include "kernel/param.h"
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
    int mask;
    char *nargv[MAXARG];

    if(argc < 3){
        fprintf(2, "usage: trace mask command [args...]\n");
        exit(1);
    }

    mask = atoi(argv[1]);
    if(trace(mask) < 0){          // 调用我们刚加的系统调用
        fprintf(2, "%s: trace failed\n", argv[0]);
        exit(1);
    }

    // 把 argv[2..] 拷到 nargv
    for(int i = 2; i < argc && i-2 < MAXARG-1; i++)
        nargv[i-2] = argv[i];
    nargv[argc-2] = 0;

    exec(nargv[0], nargv);        // 替换为要追踪的程序
    fprintf(2, "%s: exec %s failed\n", argv[0], nargv[0]);
    exit(1);
}