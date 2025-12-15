//
// Created by ZacharyXu on 2025/12/14.
//

#define MAX_ORDER 10  // 支持2^10 = 1024页 = 4MB

struct free_area {
    struct list_head free_list;  // 链表头
    unsigned long nr_free;       // 当前数量
};

struct buddy_allocator {
    struct spinlock lock;
    struct free_area free_area[MAX_ORDER+1];
    unsigned long *bitmap;  // 标记每块是否分裂
};
