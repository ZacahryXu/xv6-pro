//
// Created by lenovo on 25-12-9.
//

#ifndef SYSINFO_H
#define SYSINFO_H

struct sysinfo {
    uint64 freemem;   // 空闲物理内存字节数
    uint64 nproc;     // 活动进程数
};

#endif