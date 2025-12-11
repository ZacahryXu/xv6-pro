// kernel/vmcopyin.c
// copyin/copyinstr that use the kernel page table so that.
// the user pages don't need to be mapped.

#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

// Copy from user address `src` in page table `pagetable`
// to kernel address `dst` in the kernel page table.
// `len` bytes.
// Returns 0 on success, -1 on failure.
int
copyin_new(pagetable_t pagetable, char *dst, uint64 srcva, uint64 len)
{
//    struct proc *p = myproc();
    __attribute__((unused)) struct proc *p = myproc();
    uint64 n, va0, pa0;
    pte_t *pte;

    while(len > 0){
        va0 = PGROUNDDOWN(srcva);
        pte = walk(pagetable, va0, 0);
        if(pte == 0 || (*pte & PTE_V) == 0 || (*pte & PTE_U) == 0)
            return -1;
        pa0 = PTE2PA(*pte);
        if(srcva + len < srcva)   // overflow
            return -1;
        n = PGSIZE - (srcva - va0);
        if(n > len)
            n = len;
        memmove(dst, (void*)(pa0 + (srcva - va0)), n);

        len -= n;
        dst += n;
        srcva = va0 + PGSIZE;
    }
    return 0;
}

// Copy a null-terminated string from user address `src` in page table
// `pagetable` to kernel address `dst` in the kernel page table.
// Copy at most `max` bytes.  Return 0 on success, -1 on error.
int
copyinstr_new(pagetable_t pagetable, char *dst, uint64 srcva, uint64 max)
{
//    struct proc *p = myproc();
    __attribute__((unused)) struct proc *p = myproc();
    uint64 n, va0, pa0;
    pte_t *pte;
    int got_null;

    got_null = 0;
    while(max > 0 && !got_null){
        va0 = PGROUNDDOWN(srcva);
        pte = walk(pagetable, va0, 0);
        if(pte == 0 || (*pte & PTE_V) == 0 || (*pte & PTE_U) == 0)
            return -1;
        pa0 = PTE2PA(*pte);
        if(srcva + max < srcva)   // overflow
            return -1;
        n = PGSIZE - (srcva - va0);
        if(n > max)
            n = max;

        char *s = (char*)(pa0 + (srcva - va0));
        for(int i = 0; i < n; i++){
            if(s[i] == '\0'){
                *dst = '\0';
                got_null = 1;
                break;
            }
            *dst++ = s[i];
        }

        max -= n;
        srcva = va0 + PGSIZE;
    }
    if(!got_null)
        return -1;
    return 0;
}