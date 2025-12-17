// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

#define MAX_ORDER 10  // 最大块: 2^10 pages = 4MB

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist[MAX_ORDER + 1];
} kmem;

void
kinit()
{
  initlock(&kmem.lock, "kmem");
  for(int i = 0; i <= MAX_ORDER; i++) {
    kmem.freelist[i] = 0;
  }
  freerange(end, (void*)PHYSTOP);
}

static void
buddyfree_order(void *pa, int order)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    return;

  memset(pa, 1, (1 << order) * PGSIZE);

  while(order < MAX_ORDER) {
    uint64 buddy_addr = (uint64)pa ^ ((uint64)1 << (order + 12));

    if(buddy_addr < (uint64)end || buddy_addr >= PHYSTOP)
      break;

    struct run **pp = &kmem.freelist[order];
    struct run *r;
    int found = 0;

    while((r = *pp) != 0) {
      if((uint64)r == buddy_addr) {
        *pp = r->next;
        found = 1;
        break;
      }
      pp = &r->next;
    }

    if(!found)
      break;

    if(buddy_addr < (uint64)pa)
      pa = (void*)buddy_addr;

    order++;
  }

  r = (struct run*)pa;
  r->next = kmem.freelist[order];
  kmem.freelist[order] = r;
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);

  for(; p + ((1 << MAX_ORDER) * PGSIZE) <= (char*)pa_end;
      p += (1 << MAX_ORDER) * PGSIZE) {
    buddyfree_order(p, MAX_ORDER);
  }

  int order = MAX_ORDER - 1;
  while(order >= 0 && p < (char*)pa_end) {
    if(p + ((1 << order) * PGSIZE) <= (char*)pa_end) {
      buddyfree_order(p, order);
      p += (1 << order) * PGSIZE;
    } else {
      order--;
    }
  }
}

void
kfree(void *pa)
{
  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  acquire(&kmem.lock);
  buddyfree_order(pa, 0);
  release(&kmem.lock);
}

static void*
buddyalloc_order(int order)
{
  struct run *r;

  int current_order = order;
  while(current_order <= MAX_ORDER && !kmem.freelist[current_order]) {
    current_order++;
  }

  if(current_order > MAX_ORDER) {
    return 0;
  }

  r = kmem.freelist[current_order];
  kmem.freelist[current_order] = r->next;

  while(current_order > order) {
    current_order--;
    void *buddy_addr = (void*)((uint64)r ^ ((uint64)1 << (current_order + 12)));
    struct run *buddy_block = (struct run*)buddy_addr;
    buddy_block->next = kmem.freelist[current_order];
    kmem.freelist[current_order] = buddy_block;
  }

  memset((char*)r, 5, (1 << order) * PGSIZE);
  return (void*)r;
}

void *
kalloc(void)
{
  void *r;

  acquire(&kmem.lock);
  r = buddyalloc_order(0);
  release(&kmem.lock);

  return r;
}