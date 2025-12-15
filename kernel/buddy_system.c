#include <stddef.h>

#include "buddy.h"
//
// Created by ZacharyXu on 2025/12/15.
//
// 分配算法伪代码
void* buddy_alloc(int order) {

    for (int curr = order; curr <= MAX_ORDER; curr++) {
        if (!free_area[curr]) {
        }
    }
    return NULL;  // 内存不足
}

// 释放算法伪代码
void buddy_free(void *addr, int order) {
    while (order < MAX_ORDER) {

    }

}