#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main() {
    sleep(1);  // 调用 sys_sleep，触发 backtrace
    exit(0);
}