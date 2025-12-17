#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);
extern char end[]; // 内核代码后的第一个地址，由 kernel.ld 定义

#define MAX_ORDER 10  // 最大块: 2^10 pages = 4MB

struct run {
  struct run *next;
};

// Buddy System 结构体
struct {
  struct spinlock lock;
  struct run *freelist[MAX_ORDER + 1];
} kmem;

// COW 引用计数结构体
struct {
  struct spinlock lock;
  int count[PHYSTOP / PGSIZE];
} ref;

void
kinit()
{
  initlock(&kmem.lock, "kmem");
  initlock(&ref.lock, "ref");

  // 初始化所有 freelist 为空
  for(int i = 0; i <= MAX_ORDER; i++) {
    kmem.freelist[i] = 0;
  }

  // 初始化引用计数为 0
  for(int i = 0; i < PHYSTOP/PGSIZE; i++) {
    ref.count[i] = 0;
  }

  // 将可用内存范围交给 buddy 管理
  freerange(end, (void*)PHYSTOP);
}

// 供 COW 使用：增加物理页引用计数
void
kref_inc(void* pa)
{
  if((uint64)pa >= PHYSTOP) panic("kref_inc");
  acquire(&ref.lock);
  ref.count[(uint64)pa / PGSIZE]++;
  release(&ref.lock);
}

// 供 COW 使用：减少物理页引用计数
int
kref_dec(void* pa)
{
  if((uint64)pa >= PHYSTOP) panic("kref_dec");
  int c;
  acquire(&ref.lock);
  c = --ref.count[(uint64)pa / PGSIZE];
  release(&ref.lock);
  return c;
}

// 内部函数：向 Buddy System 释放特定阶的块
static void
buddyfree_order(void *pa, int order)
{
  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    return;

  // 这里的 memset 仅用于调试，清理释放的内存
  memset(pa, 1, (1 << order) * PGSIZE);

  while(order < MAX_ORDER) {
    // 计算伙伴地址：地址 XOR 块大小
    uint64 buddy_addr = (uint64)pa ^ ((uint64)1 << (order + 12));

    if(buddy_addr < (uint64)end || buddy_addr >= PHYSTOP)
      break;

    // 在当前 order 的链表中查找伙伴是否空闲
    struct run **pp = &kmem.freelist[order];
    struct run *r;
    int found = 0;

    while((r = *pp) != 0) {
      if((uint64)r == buddy_addr) {
        // 找到伙伴，将其从空闲链表中移除
        *pp = r->next;
        found = 1;
        break;
      }
      pp = &r->next;
    }

    if(!found)
      break; // 伙伴不空闲，停止合并

    // 合并：确保 pa 指向合并后的大块基地址
    if(buddy_addr < (uint64)pa)
      pa = (void*)buddy_addr;

    order++;
  }

  // 将合并后的最终块放入链表
  struct run *r = (struct run*)pa;
  r->next = kmem.freelist[order];
  kmem.freelist[order] = r;
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  
  // 通过逐页调用 kfree 来填充 Buddy System
  // 这样做可以确保所有页面引用计数初始化正确，且 buddy 能自动合并对齐的块
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    kfree(p);
}

// 释放物理页
void
kfree(void *pa)
{
  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // COW 逻辑：只有当引用计数减到 0 时，才真正释放内存
  if(kref_dec(pa) > 0)
    return;

  acquire(&kmem.lock);
  buddyfree_order(pa, 0); // 初始释放总是 order 0 (4KB)
  release(&kmem.lock);
}

// 内部函数：分配特定阶的块
static void*
buddyalloc_order(int order)
{
  int current_order = order;
  
  // 寻找最小可用的足够大的块
  while(current_order <= MAX_ORDER && !kmem.freelist[current_order]) {
    current_order++;
  }

  if(current_order > MAX_ORDER)
    return 0;

  struct run *r = kmem.freelist[current_order];
  kmem.freelist[current_order] = r->next;

  // 如果取出的块太大，递归拆分 (Splitting)
  while(current_order > order) {
    current_order--;
    void *buddy_addr = (void*)((uint64)r ^ ((uint64)1 << (current_order + 12)));
    struct run *buddy_block = (struct run*)buddy_addr;
    
    // 将拆出来的另一半放入低阶链表
    buddy_block->next = kmem.freelist[current_order];
    kmem.freelist[current_order] = buddy_block;
  }

  return (void*)r;
}

// 分配一个 4096 字节的物理页
void *
kalloc(void)
{
  void *r;

  acquire(&kmem.lock);
  r = buddyalloc_order(0); // 默认分配 order 0
  release(&kmem.lock);

  if(r) {
    // 成功分配后，将引用计数设为 1
    acquire(&ref.lock);
    ref.count[(uint64)r / PGSIZE] = 1;
    release(&ref.lock);
    
    memset((char*)r, 5, PGSIZE); // 填充垃圾数据
  }

  return r;
}