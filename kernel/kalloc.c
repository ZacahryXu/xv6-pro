// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.
//物理内存分配器，用于用户进程，内核栈，页表页，并且管道缓存.分配全部的4096byte页面

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.
//定义了一个链表的节点结构体，代表运行的内存页
struct run {
  struct run *next;
};
//定义了一个链表的结构体，代表未运行的内存页
struct {
  //锁
  struct spinlock lock;
  //空闲页链表
  struct run *freelist;
} kmem;
//物理页的初始化
void
kinit()
{
  //初始化锁,锁的名字是kmem,只能有一个cpu创建内存
  initlock(&kmem.lock, "kmem");
  //释放范围
  freerange(end, (void*)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{

  char *p;
  uint64 start = (uint64)pa_start;
  p = (char*)PGROUNDUP(start);
  //遍历所有页面
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    //释放页面
    kfree(p);
}

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
//物理页的资源释放
void
kfree(void *pa)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  acquire(&kmem.lock);
  //通过头插法将释放的页加入freelist头部
  //将r的下一个节点指向空闲内存页
  r->next = kmem.freelist;
  //将kmem.freelist指向的空闲页指向r
  kmem.freelist = r;
  //释放锁
  release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
//物理页分配，分配4096B的物理内存页
//返回一个内核可以使用的指针
//如果内存不能分配返回0
void *
kalloc(void)
{
  struct run *r;
  //获取锁
  acquire(&kmem.lock);
  //把剩余的空闲内存页的链表赋值给r
  r = kmem.freelist;
  // printf("alloc %p\n", r);
  if(r)
    //指针后移
    kmem.freelist = r->next;
  release(&kmem.lock);

  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk  使用垃圾填充
  return (void*)r;
}
