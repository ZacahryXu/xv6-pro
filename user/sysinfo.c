//
// Created by lenovo on 25-12-9.
//

#include "kernel/types.h"
#include "kernel/sysinfo.h"
#include "user/user.h"

int
main(void)
{
    struct sysinfo info;

    if (sysinfo(&info) < 0) {
        printf("sysinfo: failed\n");
        exit(1);
    }
    printf("free memory: %ld bytes\nprocesses: %ld\n",
           info.freemem, info.nproc);
    exit(0);
}