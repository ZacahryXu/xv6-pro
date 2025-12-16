#include "kernel/param.h"
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

void alarm_handler(void){
    write(1, "alarm!\n", 7);
    sigreturn();          // 立即返回，不许再干别的
}

int
main(int argc, char *argv[])
{
    sigalarm(2, alarm_handler);   // 纯 C 传参，无冒号
    while(1){
        // 空转
    }
}